/**============================================================================
Name        : binance_market_data_source.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market data source implementation.
============================================================================**/

#include "binance_market_data_source.hpp"
#include <iostream>

namespace
{
    const std::string wsStreamHost { "stream.binance.com" };
    constexpr uint16_t port { 443 };
}

namespace trading::exchanges::binance
{
    BinanceMarketDataSource::BinanceMarketDataSource(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) },
        ioContext { 1 },
        sslContext { boost::asio::ssl::context::tls_client },
        wsClient    {
            ioContext, sslContext,wsStreamHost,std::to_string(port),"/ws"
        }
    {
        sslContext.set_default_verify_paths();
        sslContext.set_verify_mode(boost::asio::ssl::verify_peer);
    }

    void BinanceMarketDataSource::start()
    {
        wsClient.start([](const std::string& message) {
            std::cout << message << '\n';
        });

        wsClient.send(R"({"method":"SUBSCRIBE","params":["btcusdt@depth", "btcusdt@trade"],"id":1})");
        ioContext.run();
    }

    void BinanceMarketDataSource::stop()
    {
        wsClient.stop();
        ioContext.stop();
    }

    void BinanceMarketDataSource::setMessageHandler(market_data::IMarketDataMessageHandler& handler)
    {
        messageHandler = &handler;
    }
}