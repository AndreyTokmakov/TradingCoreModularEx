/**============================================================================
Name        : binance_market_data_source.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market data source implementation.
============================================================================**/

#include "binance_market_data_source.hpp"

namespace
{
    constexpr uint16_t Port { 443 };
    constexpr std::string Target = "/ws";
}

namespace trading::exchanges::binance
{
    BinanceMarketDataSource::BinanceMarketDataSource(std::string endpoint) noexcept :
        ioContext { 1 },
        sslContext { boost::asio::ssl::context::tls_client },
        wsClient    {
            ioContext, sslContext,std::move(endpoint) ,std::to_string(Port),Target
        }
    {
        sslContext.set_default_verify_paths();
        sslContext.set_verify_mode(boost::asio::ssl::verify_peer);
    }

    void BinanceMarketDataSource::start()
    {
        wsClient.start([this](const std::string& message) {
            messageHandler->onMessage(message);
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