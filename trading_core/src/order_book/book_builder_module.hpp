/**============================================================================
Name        : book_builder_module.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Processes market-data book updates on the BookBuilder thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_MODULE_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_MODULE_HPP

#include "book_builder.hpp"
#include "config/config.hpp"
#include "common/condition_variable_queue.hpp"
#include "common/worker.hpp"
#include "market_data/market_event_dispatcher.hpp"
#include "market_data/interfaces/market_data_parser.hpp"
#include "market_data/interfaces/snapshot_provider.hpp"
#include "order_book.hpp"
#include "recording/recording_event.hpp"
#include "exchanges/exchange_factory.hpp"
#include "metrics/metrics_collector.hpp"
#include "logging/logger.hpp"

namespace trading::order_book
{
    using market_data::BookUpdates;
    using market_data::MarketEvent;
    using market_data::OrderBookLevels;
    using market_data::ISnapshotProvider;
    using market_data::MarketEventDispatcher;

    class BookBuilderModule final: public common::Worker<BookBuilderModule>
    {
    public:
        BookBuilderModule(const config::Config& config,
                          concurrency::ConditionVariableQueue<BookUpdates>& bookUpdateQueue,
                          concurrency::ConditionVariableQueue<MarketEvent>& strategyEventQueue,
                          concurrency::ConditionVariableQueue<recording::RecordingEvent>& recordingQueue,
                          const exchanges::IExchangeFactory& exchangeFactory) noexcept;

        void run();

    private:
        concurrency::ConditionVariableQueue<BookUpdates>& bookUpdateQueue;

        OrderBook orderBook;
        MarketEventDispatcher marketEventDispatcher;
        BookBuilder bookBuilder;
        std::unique_ptr<ISnapshotProvider> snapshotProvider;

        std::shared_ptr<logging::ILogger> logger;
        static inline thread_local metrics::Metrics& metrics  = metrics::MetricsCollector::getCollector().getThreadLocalMetrics();
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_MODULE_HPP