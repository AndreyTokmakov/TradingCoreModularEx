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

#include <iostream>

#include <simdjson.h>
// #include <nlohmann/json.hpp>


namespace
{
    constexpr std::int64_t DecimalScale = 100'000'000;

    std::int64_t parseDecimal(const std::string_view value)
    {
        bool negative = false;
        std::size_t index = 0;

        if (!value.empty() && value.front() == '-')
        {
            negative = true;
            index = 1;
        }

        std::int64_t integerPart = 0;

        while (index < value.size() && value[index] != '.')
        {
            integerPart =
                integerPart * 10 +
                static_cast<std::int64_t>(value[index] - '0');

            ++index;
        }

        std::int64_t fractionalPart = 0;
        std::size_t fractionalDigits = 0;

        if (index < value.size() && value[index] == '.')
        {
            ++index;
            while (index < value.size() && fractionalDigits < 8)
            {
                fractionalPart = fractionalPart * 10 + static_cast<std::int64_t>(value[index] - '0');
                ++fractionalDigits;
                ++index;
            }
        }

        while (fractionalDigits < 8)
        {
            fractionalPart *= 10;
            ++fractionalDigits;
        }

        const std::int64_t result = integerPart * DecimalScale + fractionalPart;

        return negative ? -result : result;
    }
}

namespace trading::exchanges::binance
{
    market_data::ParseResult
    BinanceMarketDataParser::parse(std::string_view message,
                                   [[maybe_unused]] market_data::MarketDataItem& marketDataItem) const
    {
        simdjson::dom::parser parser;
        const simdjson::dom::element document = parser.parse(message.data(), message.size());

        market_data::BookUpdate bookUpdate;

        bookUpdate.exchangeTimestamp   = static_cast<Timestamp>(document["E"].get_uint64());
        bookUpdate.sequenceRange.first = static_cast<SequenceNumber>(document["U"].get_uint64());
        bookUpdate.sequenceRange.last  = static_cast<SequenceNumber>(document["u"].get_uint64());

        const auto& bids = document["b"].get_array();
        const auto& asks = document["a"].get_array();
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

        return market_data::ParseResult::Success;
    }
}