/**============================================================================
Name        : binance_market_data_source.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market data source implementation.
============================================================================**/

#include "binance_market_data_source.hpp"

#include <utility>

namespace trading::exchanges::binance
{
    BinanceMarketDataSource::BinanceMarketDataSource(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) }
    {
    }

    void BinanceMarketDataSource::start()
    {
        running = true;

        // TODO: Connect to endpoint.
        // TODO: Start receiving market data.
    }

    void BinanceMarketDataSource::stop()
    {
        running = false;

        // TODO: Close connection.
    }

    void BinanceMarketDataSource::setMessageHandler(market_data::IMarketDataMessageHandler& handler)
    {
        messageHandler = &handler;
    }
}