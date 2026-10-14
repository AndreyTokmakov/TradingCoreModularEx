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

#include "interfaces/market_data_parser.hpp"
#include "interfaces/market_data_source.hpp"
#include "market_data_message_handler.hpp"
#include "exchanges/exchange_factory.hpp"
#include "model/trade.hpp"

namespace trading::market_data
{
    class MarketDataModule final
    {
    public:
        MarketDataModule(const config::Config& config,
                         common::Queue<BookUpdate>& bookUpdateQueue,
                         common::Queue<Trade>& tradeQueue,
                         const exchanges::IExchangeFactory& exchangeFactory) noexcept;

        MarketDataModule(const MarketDataModule&) = delete;
        MarketDataModule& operator=(const MarketDataModule&) = delete;

        MarketDataModule(MarketDataModule&&) = delete;
        MarketDataModule& operator=(MarketDataModule&&) = delete;

        void start() const;
        void stop() const;

    private:
        std::unique_ptr<IMarketDataParser> marketDataParser;
        std::unique_ptr<IMarketDataSource> marketDataSource;
        MarketDataMessageHandler messageHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MODULE_HPP