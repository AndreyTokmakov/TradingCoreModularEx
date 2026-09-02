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
#include "config.hpp"
#include "condition_variable_queue.hpp"
#include "market_event_dispatcher.hpp"
#include "order_book.hpp"
#include "recording_event.hpp"
#include "interfaces/market_data_parser.hpp"
#include "interfaces/snapshot_provider.hpp"
#include "worker.hpp"
#include "exchange_factory.hpp"

namespace trading::market_data
{
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
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_MODULE_HPP