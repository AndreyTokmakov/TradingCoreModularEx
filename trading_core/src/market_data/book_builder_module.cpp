/**============================================================================
Name        : book_builder_module.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Processes market-data book updates on the BookBuilder thread.
============================================================================**/

#include "book_builder_module.hpp"
#include "config_utils.hpp"
#include "binance_snapshot_provider.hpp"

namespace trading::market_data
{

    BookBuilderModule::BookBuilderModule(const config::Config& config,
                                         concurrency::ConditionVariableQueue<BookUpdates>& bookUpdateQueue,
                                         concurrency::ConditionVariableQueue<MarketEvent>& strategyEventQueue,
                                         concurrency::ConditionVariableQueue<recording::RecordingEvent>& recordingQueue) noexcept :
        bookUpdateQueue { bookUpdateQueue },
        orderBook {},
        marketEventDispatcher {
            strategyEventQueue,recordingQueue
        },
        bookBuilder {
            config.instrument, orderBook, marketEventDispatcher
        },
        snapshotProvider {
            std::make_unique<exchanges::binance::BinanceSnapshotProvider>(config::findExchange(config, "binance").marketDataEndpoint)
        }
    {
    }

    void BookBuilderModule::run()
    {
        const Snapshot snapshot = snapshotProvider->getSnapshot();
        if (!bookBuilder.applySnapshot(snapshot))
            return;

        BookUpdates updates;
        while (bookUpdateQueue.waitPop(updates))
        {
            for (const BookUpdate& update : updates) {
                bookBuilder.onBookUpdate(update);
            }
            updates.clear();
        }
    }
}
