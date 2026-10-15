/**============================================================================
Name        : web_socket_client.hpp
Created on  : 15.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : web_socket_client.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_WEB_SOCKET_CLIENT_HPP
#define TRADINGCOREMODULAREX_WEB_SOCKET_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

#include <deque>
#include <functional>
#include <string>
#include <string_view>

namespace trading::network
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;

    using tcp = asio::ip::tcp;
    using error_code = beast::error_code;

    class WebSocketClient
    {
    public:
        using MessageHandler = std::function<void(std::string_view)>;
        using ErrorHandler = std::function<void(error_code)>;

        WebSocketClient(asio::io_context& ioContext,
                        asio::ssl::context& sslContext,
                        MessageHandler messageHandler,
                        ErrorHandler errorHandler = {});

        WebSocketClient(const WebSocketClient&) = delete;
        WebSocketClient& operator=(const WebSocketClient&) = delete;
        WebSocketClient(WebSocketClient&&) = delete;
        WebSocketClient& operator=(WebSocketClient&&) = delete;

        ~WebSocketClient() = default;

        void connect(std::string hostName,
                     std::string port,
                     std::string requestTarget);
        void send(std::string_view message);
        void close();

    private:
        using Strand = asio::strand<asio::io_context::executor_type>;
        using WebSocketStream = websocket::stream<beast::ssl_stream<tcp::socket>>;

        enum class State : std::uint8_t
        {
            Disconnected,
            Resolving,
            Connecting,
            TlsHandshake,
            WebSocketHandshake,
            Connected,
            Closing
        };

        void onResolve(const error_code& errorCode,
                       const tcp::resolver::results_type& results);

        void onConnect(const error_code& errorCode);
        void onSslHandshake(const error_code& errorCode);
        void onWebSocketHandshake(const error_code& errorCode);

        void read();
        void onRead(const error_code& errorCode);

        void write();
        void onWrite(const error_code& errorCode);

        void onClose(const error_code& errorCode);
        void beginClose();
        void handleFailure(const error_code& errorCode);

        void notifyError(const error_code& errorCode);

    private:

        Strand strand;
        tcp::resolver resolver;
        WebSocketStream webSocket;
        beast::flat_buffer readBuffer;

        std::string host;
        std::string target;

        std::deque<std::string> writeQueue;

        MessageHandler messageHandler;
        ErrorHandler errorHandler;

        State state { State::Disconnected };
        bool writeInProgress { false };
        bool errorReported { false };
    };
}

#endif //TRADINGCOREMODULAREX_WEB_SOCKET_CLIENT_HPP
