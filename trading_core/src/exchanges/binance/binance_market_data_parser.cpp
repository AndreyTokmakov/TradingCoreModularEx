/**============================================================================
Name        : binance_market_data_parser.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : binance_market_data_parser.cpp
============================================================================**/

/*
    BinanceMarketDataParser implementation.

    Data Flow:

        Binance raw message
               |
               v
        BinanceMarketDataParser
               |
               +----------------------+
               |                      |
               v                      v
        vector<BookUpdate>        ParseError
               |
               v
        MarketDataMessageHandler
               |
               v
          BookBuilder

    Binance-specific JSON parsing remains entirely inside this class.
*/

#include "binance_market_data_parser.hpp"
#include <simdjson.h>

namespace
{
    constexpr std::int64_t DecimalScale = 100'000'000;
    constexpr std::uint64_t NanosecondsPerMillisecond = 1'000'000;

    std::int64_t parseDecimal(const std::string_view value)
    {
        bool negative = false;
        std::size_t index = 0;

        if (!value.empty() && value.front() == '-'){
            negative = true;
            index = 1;
        }

        std::int64_t integerPart = 0;
        while (index < value.size() && value[index] != '.') {
            integerPart = integerPart * 10 + static_cast<std::int64_t>(value[index] - '0');
            ++index;
        }

        std::int64_t fractionalPart = 0;
        std::size_t fractionalDigits = 0;

        if (index < value.size() && value[index] == '.')
        {
            ++index;
            while (index < value.size() && fractionalDigits < 8) {
                fractionalPart = fractionalPart * 10 + static_cast<std::int64_t>(value[index] - '0');
                ++fractionalDigits;
                ++index;
            }
        }

        while (fractionalDigits < 8) {
            fractionalPart *= 10;
            ++fractionalDigits;
        }

        const std::int64_t result = integerPart * DecimalScale + fractionalPart;
        return negative ? -result : result;
    }

    constexpr std::string_view TradeEventType { "trade" };
    constexpr std::string_view DepthUpdateEventType { "depthUpdate" };

    // TODO: where to store ??
    simdjson::dom::parser parser;

    // TODO: create CV sonstants?
    // "t" "T" "p" "q"
}

namespace trading::exchanges::binance
{
    market_data::ParseResult
    BinanceMarketDataParser::parse(const std::string_view message,
                                   market_data::MarketDataItem& marketDataItem) const
    {
        const simdjson::dom::element document = parser.parse(message.data(), message.size());
        if (0 == sessionId) {
            sessionId = document["id"].get_uint64().value();
            return {};
        }

        const std::string_view eventType = document["e"].get_string().value();
        if (eventType == TradeEventType)
        {
            const std::uint64_t tradeId = document["t"].get_uint64().value();
            const std::uint64_t timestampMilliseconds = document["T"].get_uint64().value();
            const std::string_view priceString = document["p"].get_string().value();
            const std::string_view quantityString = document["q"].get_string().value();
            const bool buyerIsMaker = document["m"].get_bool().value();

            market_data::Trade trade;

            // trade.instrument = instrument;
            trade.sequence = tradeId;
            trade.exchangeTimestamp = Timestamp { timestampMilliseconds * NanosecondsPerMillisecond };
            trade.price = Price { parseDecimal(priceString) };
            trade.quantity = Quantity { parseDecimal(quantityString) };
            trade.side = buyerIsMaker ? Side::Sell : Side::Buy;

            // FIXME: Optimize  ???
            marketDataItem.emplace<market_data::Trades>({trade});
        }
        else if (eventType == DepthUpdateEventType)
        {
            market_data::BookUpdate bookUpdate;

            bookUpdate.exchangeTimestamp   = static_cast<Timestamp>(document["E"].get_uint64());
            bookUpdate.sequenceRange.first = static_cast<SequenceNumber>(document["U"].get_uint64());
            bookUpdate.sequenceRange.last  = static_cast<SequenceNumber>(document["u"].get_uint64());

            const auto& bids = document["b"].get_array();
            const auto& asks = document["a"].get_array();

            // FIXME: Reusable vector pool?
            bookUpdate.updates.reserve(bids.size() + asks.size());

            for (const auto& level : bids)
            {
                const simdjson::dom::array values = level.get_array();
                bookUpdate.updates.emplace_back(
                    Side::Buy,
                    Price { parseDecimal(values.at(0).get_string()) },
                    Quantity { parseDecimal( values.at(1).get_string()) }
                );
            }
            for (const auto& level : asks)
            {
                const simdjson::dom::array values = level.get_array();
                bookUpdate.updates.emplace_back(
                    Side::Sell,
                    Price { parseDecimal(values.at(0).get_string()) },
                    Quantity { parseDecimal( values.at(1).get_string()) }
                );
            }

            // FIXME: Optimize  ???
            marketDataItem = std::move(bookUpdate);
        }

        return market_data::ParseResult::Success;
    }
}