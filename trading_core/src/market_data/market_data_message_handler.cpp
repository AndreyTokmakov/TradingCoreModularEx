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
#include "logging/logger_factory.hpp"

namespace trading::market_data
{
    MarketDataMessageHandler::MarketDataMessageHandler(IMarketDataParser& parser,
                                                       concurrency::Queue<BookUpdate>& bookUpdateQueue,
                                                       concurrency::Queue<Trade>& tradeQueue) noexcept:
        parser { parser },
        bookUpdateQueue { bookUpdateQueue },
        tradeQueue { tradeQueue },
        logger { logging::LoggerFactory::getLogger() }
    {
    }

    void MarketDataMessageHandler::onMessage(const std::string_view message)
    {
        marketDataItem.emplace<std::monostate>();
        if (parser.parse(message, marketDataItem) != ParseResult::Success) {
            logger->error("Failed to parse market data message");
            metrics.increment<metrics::MetricType::MarketDataParseErrors>();
            return;
        }

        metrics.increment<metrics::MetricType::MarketDataUpdates>();
        if (BookUpdate* bookUpdate = std::get_if<BookUpdate>(&marketDataItem))
        {
            if (!bookUpdate->empty())
                bookUpdateQueue.push(std::move(*bookUpdate));
        }
        else if (const Trades* trades = std::get_if<Trades>(&marketDataItem))
        {
            for (const Trade& trade : *trades)
                tradeQueue.push(trade);
        }
    }
}