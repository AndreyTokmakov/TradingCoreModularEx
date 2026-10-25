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

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include <functional>
#include <memory>
#include <string>

namespace trading::network
{
    class WebSocketSession;

    class WebSocketClient
    {
    public:
        using MessageHandler = std::function<void(std::string)>;

        WebSocketClient(boost::asio::io_context& ioContext,
                        boost::asio::ssl::context& sslContext,
                        std::string host,
                        std::string port,
                        std::string target);
        ~WebSocketClient();

        WebSocketClient(const WebSocketClient&) = delete;
        WebSocketClient& operator=(const WebSocketClient&) = delete;

        void start(MessageHandler handler);
        void send(std::string message) const;
        void stop();

    private:
        boost::asio::io_context& ioContext;
        boost::asio::ssl::context& sslContext;
        std::string host;
        std::string port;
        std::string target;
        std::shared_ptr<WebSocketSession> session;
    };
}

#endif //TRADINGCOREMODULAREX_WEB_SOCKET_CLIENT_HPP
