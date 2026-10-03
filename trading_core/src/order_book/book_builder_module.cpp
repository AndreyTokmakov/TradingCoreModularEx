/**============================================================================
Name        : book_builder_module.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Processes market-data book updates on the BookBuilder thread.
============================================================================**/

#include "book_builder_module.hpp"
#include "config/config_utils.hpp"
#include "logging/logger_factory.hpp"

namespace trading::order_book
{
    BookBuilderModule::BookBuilderModule(const config::Config& config,
                                         concurrency::ConditionVariableQueue<BookUpdate>& bookUpdateQueue,
                                         concurrency::ConditionVariableQueue<MarketEvent>& strategyEventQueue,
                                         concurrency::ConditionVariableQueue<recording::RecordingEvent>& recordingQueue,
                                         const exchanges::IExchangeFactory& exchangeFactory) noexcept :
        bookUpdateQueue { bookUpdateQueue },
        orderBook { config.orderBook.depthValue },
        marketEventDispatcher {
            strategyEventQueue,recordingQueue
        },
        bookBuilder {
            config.instrument, orderBook, marketEventDispatcher
        },
        snapshotProvider {
            exchangeFactory.createSnapshotProvider(config)
        },
        logger {
            logging::LoggerFactory::getLogger()
        }
    {
    }

    void BookBuilderModule::run()
    {
        const Snapshot snapshot = snapshotProvider->getSnapshot();
        metrics.increment<metrics::MetricType::MarketDataSnapshots>();

        if (!bookBuilder.applySnapshot(snapshot)) {
            logger->error("Failed to apply book builder snapshot");
            metrics.increment<metrics::MetricType::MarketDataSnapshotApplyFailed>();
            return;
        }

        BookUpdate bookUpdate;
        while (bookUpdateQueue.waitPop(bookUpdate))
        {
            bookBuilder.onBookUpdate(bookUpdate);
            bookUpdate.clear();
        }
    }
}
