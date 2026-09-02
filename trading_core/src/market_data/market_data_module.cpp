/**============================================================================
Name        : market_data_module.cpp
Created on  : 30.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market-data pipeline module implementation.
============================================================================**/

#include "market_data_module.hpp"

namespace trading::market_data
{
    MarketDataModule::MarketDataModule(const config::Config& config,
                                       concurrency::Queue<BookUpdates>& bookUpdateQueue,
                                       const exchanges::IExchangeFactory& exchangeFactory) noexcept:
        marketDataParser {
            exchangeFactory.createMarketDataParser(config)
        },
        marketDataSource {
            exchangeFactory.createMarketDataSource(config)
        },
        messageHandler { *marketDataParser, bookUpdateQueue }
    {
        marketDataSource->setMessageHandler(messageHandler);
    }

    void MarketDataModule::start()
    {
        marketDataSource->start();
    }

    void MarketDataModule::stop()
    {
        marketDataSource->stop();
    }
}