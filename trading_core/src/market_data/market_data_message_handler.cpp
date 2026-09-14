/**============================================================================
Name        : market_data_message_handler.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_data_message_handler.cpp
============================================================================**/

/*
    MarketDataMessageHandler implementation.

    Data Flow:

        Raw Market Data
               |
               v
        MarketDataMessageHandler
               |
               | parse(message, bookUpdates)
               v
        IMarketDataParser
               |
               | fills reusable buffer
               v
        BookUpdates
               |
               v
        BookBuilder

    The BookUpdates buffer is reused between messages to avoid allocations
    on the market-data hot path.
*/

#include "market_data_message_handler.hpp"
#include "logger_factory.hpp"

namespace trading::market_data
{
    MarketDataMessageHandler::MarketDataMessageHandler(IMarketDataParser& parser,
                                                       concurrency::Queue<BookUpdates>& bookUpdateQueue) noexcept:
        parser { parser },
        bookUpdateQueue { bookUpdateQueue },
        logger { logging::LoggerFactory::getLogger() }
    {
    }

    void MarketDataMessageHandler::onMessage(const std::string_view message)
    {
        bookUpdates.clear();

        if (parser.parse(message, bookUpdates) != ParseResult::Success) {
            logger->error("Failed to parse book updates from message");
            metrics.increment<metrics::MetricType::MarketDataParseErrors>();
            return;
        }

        metrics.increment<metrics::MetricType::MarketDataUpdates>();
        if (!bookUpdates.empty())
            bookUpdateQueue.push(std::move(bookUpdates));
    }
}