/**============================================================================
Name        : market_data_module.cpp
Created on  : 30.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market-data pipeline module implementation.
============================================================================**/

#include "market_data_module.hpp"

#include <utility>

namespace trading::market_data
{
    MarketDataModule::MarketDataModule(std::string endpoint,
                                       concurrency::Queue<BookUpdates>& bookUpdateQueue) noexcept:
        parser {},
        messageHandler { parser, bookUpdateQueue },
        source { std::move(endpoint) }
    {
        source.setMessageHandler(messageHandler);
    }

    void MarketDataModule::start()
    {
        source.start();
    }

    void MarketDataModule::stop()
    {
        source.stop();
    }
}