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
                                       common::Queue<BookUpdate>& bookUpdateQueue,
                                       common::Queue<Trade>& tradeQueue,
                                       const exchanges::IExchangeFactory& exchangeFactory) noexcept:
        marketDataParser {
            exchangeFactory.createMarketDataParser(config)
        },
        marketDataSource {
            exchangeFactory.createMarketDataSource(config)
        },
        messageHandler { *marketDataParser, bookUpdateQueue, tradeQueue }
    {
        marketDataSource->setMessageHandler(messageHandler);
    }

    void MarketDataModule::start() const
    {
        marketDataSource->start();
    }

    void MarketDataModule::stop() const
    {
        marketDataSource->stop();
    }
}