/**============================================================================
Name        : web_socket_client.cpp
Created on  : 15.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : web_socket_client.cpp
============================================================================**/

#include "web_socket_client.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <boost/asio/ssl.hpp>
//#include <boost/asio/ssl/host_name_verification.hpp>

#include <utility>

namespace trading::network
{
    WebSocketClient::WebSocketClient(asio::io_context& ioContext,
                                     asio::ssl::context& sslContext,
                                     MessageHandler messageHandler,
                                     ErrorHandler errorHandler):
        strand { asio::make_strand(ioContext) },
        resolver { strand },
        webSocket { strand, sslContext },
        messageHandler { std::move(messageHandler) },
        errorHandler { std::move(errorHandler) }
    {
        webSocket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
    }

    void WebSocketClient::connect(std::string hostName, std::string port, std::string requestTarget)
    {
        asio::dispatch(strand, [this, hostName = std::move(hostName), port = std::move(port),
                                requestTarget = std::move(requestTarget)]() mutable
        {
            if (state != State::Disconnected) {
                return;
            }

            host = std::move(hostName);
            target = std::move(requestTarget);

            errorReported = false;
            state = State::Resolving;

            resolver.async_resolve(host, port, [this](const error_code& errorCode, const tcp::resolver::results_type& results) {
                onResolve(errorCode, results);
            });
        });
    }

    void WebSocketClient::send(const std::string_view message)
    {
        std::string ownedMessage { message };
        asio::dispatch(strand, [this, message = std::move(ownedMessage)] {
            if (state == State::Disconnected || state == State::Closing) {
                return;
            }

            writeQueue.push_back(std::move(message));
            if (state == State::Connected) {
                write();
            }
        });
    }

    void WebSocketClient::close()
    {
        asio::dispatch(strand, [this]
        {
            if (state == State::Disconnected || state == State::Closing) {
                return;
            }

            if (state != State::Connected)
            {
                state = State::Disconnected;
                resolver.cancel();

                error_code ignored;
                webSocket.next_layer().next_layer().cancel(ignored);
                webSocket.next_layer().next_layer().close(ignored);

                // No write can be active before the WebSocket handshake completes.
                writeQueue.clear();

                return;
            }

            state = State::Closing;
            if (!writeInProgress) {
                beginClose();
            }
        });
    }

    void WebSocketClient::onResolve(const error_code& errorCode,
                                    const tcp::resolver::results_type& results)
    {
        if (state != State::Resolving) {
            return;
        }
        if (errorCode){
            handleFailure(errorCode);
            return;
        }

        state = State::Connecting;
        asio::async_connect(webSocket.next_layer().next_layer(), results, [this](const error_code& errCode, const tcp::endpoint&){
            onConnect(errCode);
        });
    }

    void WebSocketClient::onConnect(const error_code& errorCode)
    {
        if (state != State::Connecting) {
            return;
        }
        if (errorCode) {
            handleFailure(errorCode);
            return;
        }
        if (!SSL_set_tlsext_host_name(webSocket.next_layer().native_handle(), host.c_str())) {
            const error_code sslError { static_cast<int>(ERR_get_error()), asio::error::get_ssl_category() };
            handleFailure(sslError);
            return;
        }

#if 0   // Prefer the modern host_name_verification API when the build environment supports it.
        // Currently disabled because the compiler cannot resolve this API in the current
        // Boost.Asio setup, despite using Boost 1.88.0. The exact cause may be related to
        // the mixed system and FetchContent Boost headers.
        webSocket.next_layer().set_verify_callback(asio::ssl::host_name_verification(host));
#else
        // Fallback to the deprecated RFC 2818 verification API for compatibility with the
        // current build environment. It performs hostname verification against the server
        // certificate and is retained until the Boost.Asio include/version mismatch is fixed.
        webSocket.next_layer().set_verify_callback(asio::ssl::rfc2818_verification(host));
#endif

        state = State::TlsHandshake;
        webSocket.next_layer().async_handshake(asio::ssl::stream_base::client, [this](const error_code& errCode) {
            onSslHandshake(errCode);
        });
    }

    void WebSocketClient::onSslHandshake(const error_code& errorCode)
    {
        if (state != State::TlsHandshake) {
            return;
        }
        if (errorCode) {
            handleFailure(errorCode);
            return;
        }

        state = State::WebSocketHandshake;
        webSocket.async_handshake(host, target, [this](const error_code& errCode) {
            onWebSocketHandshake(errCode);
        });
    }

    void WebSocketClient::onWebSocketHandshake(const error_code& errorCode)
    {
        if (state != State::WebSocketHandshake) {
            return;
        }
        if (errorCode) {
            handleFailure(errorCode);
            return;
        }

        state = State::Connected;

        read();
        write();
    }

    void WebSocketClient::read()
    {
        if (state != State::Connected) {
            return;
        }
        webSocket.async_read(readBuffer,[this](const error_code& errorCode, std::size_t) {
            onRead(errorCode);
        });
    }

    void WebSocketClient::onRead(const error_code& errorCode)
    {
        if (errorCode)
        {
            if (errorCode == websocket::error::closed) {
                onClose(errorCode);
            } else {
                handleFailure(errorCode);
            }

            return;
        }

        const auto data = readBuffer.data();
        const std::string_view message { static_cast<const char*>(data.data()), readBuffer.size() };

        if (messageHandler) {
            messageHandler(message);
        }
        readBuffer.consume(readBuffer.size());
        read();
    }

    void WebSocketClient::write()
    {
        if (state != State::Connected || writeInProgress || writeQueue.empty()){
            return;
        }

        writeInProgress = true;
        webSocket.text(true);

        // Keep the front string alive until onWrite() completes.
        webSocket.async_write(asio::buffer(writeQueue.front()), [this](const error_code& errorCode, std::size_t) {
            onWrite(errorCode);
        });
    }

    void WebSocketClient::onWrite(const error_code& errorCode)
    {
        writeInProgress = false;

        // A read-side failure or remote close may have changed the state while
        // async_write() was outstanding. Its buffer is safe to release now.
        if (state == State::Disconnected) {
            writeQueue.clear();
            return;
        }

        if (errorCode) {
            handleFailure(errorCode);
            return;
        }

        if (!writeQueue.empty()) {
            writeQueue.pop_front();
        }

        if (state == State::Closing) {
            beginClose();
            return;
        }

        write();
    }

    void WebSocketClient::beginClose()
    {
        if (state != State::Closing || writeInProgress) {
            return;
        }

        // No async_write() is using the queue now.
        writeQueue.clear();

        webSocket.async_close(websocket::close_code::normal,
            [this](const error_code& errorCode)
            {
                onClose(errorCode);
            });
    }

    void WebSocketClient::onClose(const error_code& errorCode)
    {
        state = State::Disconnected;

        // An outstanding async_write() may still reference writeQueue.front().
        // Leave the queue intact until its completion handler runs.
        if (!writeInProgress) {
            writeQueue.clear();
        }

        if (errorCode && errorCode != websocket::error::closed && errorCode != asio::error::operation_aborted)
        {
            notifyError(errorCode);
        }
    }

    void WebSocketClient::handleFailure(const error_code& errorCode)
    {
        if (state == State::Disconnected)
        {
            return;
        }

        state = State::Disconnected;
        resolver.cancel();

        error_code ignored;
        webSocket.next_layer().next_layer().cancel(ignored);
        webSocket.next_layer().next_layer().close(ignored);

        // Do not invalidate the active write buffer before its callback.
        if (!writeInProgress) {
            writeQueue.clear();
        }

        notifyError(errorCode);
    }

    void WebSocketClient::notifyError(const error_code& errorCode)
    {
        if (errorReported) {
            return;
        }

        errorReported = true;
        if (errorHandler) {
            errorHandler(errorCode);
        }
    }
}