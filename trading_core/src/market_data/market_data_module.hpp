/**============================================================================
Name        : market_data_module.hpp
Created on  : 30.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market-data pipeline module.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MODULE_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MODULE_HPP

#include "binance_market_data_parser.hpp"
#include "binance_market_data_source.hpp"
#include "condition_variable_queue.hpp"
#include "market_data_message_handler.hpp"
#include "model/book_update.hpp"

#include <string>

namespace trading::market_data
{
    class MarketDataModule final
    {
    public:
        MarketDataModule(std::string endpoint,
                         concurrency::Queue<BookUpdates>& bookUpdateQueue) noexcept;

        MarketDataModule(const MarketDataModule&) = delete;
        MarketDataModule& operator=(const MarketDataModule&) = delete;

        MarketDataModule(MarketDataModule&&) = delete;
        MarketDataModule& operator=(MarketDataModule&&) = delete;

        void start();
        void stop();

    private:
        exchanges::binance::BinanceMarketDataParser parser; // TODO: --> std::unique_ptr< market_data::IMarketDataParser>
        MarketDataMessageHandler messageHandler;
        exchanges::binance::BinanceMarketDataSource source; // TODO: --> std::unique_ptr< market_data::IMarketDataSource>
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MODULE_HPP