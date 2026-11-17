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
    BinanceMarketDataSource::BinanceMarketDataSource(std::string endpoint,
                                                     networking::NetworkRuntime& networkRuntime) noexcept :
        wsClient    {
            networkRuntime.getIoContext(), networkRuntime.getSslContext(),std::move(endpoint) ,std::to_string(Port),Target
        }
    {
    }

    void BinanceMarketDataSource::start()
    {
        wsClient.start([this](const std::string& message) {
            messageHandler->onMessage(message);
        });
        wsClient.send(R"({"method":"SUBSCRIBE","params":["btcusdt@depth", "btcusdt@trade"],"id":1})");
    }

    void BinanceMarketDataSource::stop()
    {
        wsClient.stop();
    }

    void BinanceMarketDataSource::setMessageHandler(market_data::IMarketDataMessageHandler& handler)
    {
        messageHandler = &handler;
    }
}