/**============================================================================
Name        : market_data_module.cpp
Created on  : 30.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market-data pipeline module implementation.
============================================================================**/

#include "config_utils.hpp"
#include "market_data_module.hpp"
#include "binance_market_data_parser.hpp"
#include "binance_market_data_source.hpp"


namespace trading::market_data
{
    namespace binance = exchanges::binance;

    MarketDataModule::MarketDataModule(const config::Config& config,
                                       concurrency::Queue<BookUpdates>& bookUpdateQueue) noexcept:
        marketDataParser { std::make_unique<binance::BinanceMarketDataParser>() },
        marketDataSource { std::make_unique<binance::BinanceMarketDataSource>(config::findExchange(config, "binance").marketDataEndpoint) },
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