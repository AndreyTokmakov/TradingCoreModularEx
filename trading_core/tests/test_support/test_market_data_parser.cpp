/**============================================================================
Name        : test_market_data_parser.сpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_market_data_parser.сpp
============================================================================**/

#include "test_market_data_parser.hpp"
#include "debug_helpers.hpp"

#include <string_view>
#include <vector>
#include <charconv>

namespace
{
    template<typename T>
    bool parseNumber(std::string_view str, T& out)
    {
        if (str.empty())
            return false;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
        return ec == std::errc() && ptr == str.data() + str.size();
    }

}

namespace trading::testing
{
    using BookUpdate = market_data::BookUpdate;
    using ParseResult = market_data::ParseResult;

    ParseResult
    TestMarketDataParser::parse(const std::string_view message,
                                market_data::MarketDataItem& marketDataItem) const
    {
        // NOTE: Currently just BookUpdate parsing supported --> need to support Trade's
        const ParseResult result = parseBookUpdate(message, bookUpdatesCached);
        if (result != ParseResult::Success) {
            marketDataItem.emplace<std::monostate>();
            return result;
        }

        marketDataItem.emplace<BookUpdate>(bookUpdatesCached);
        return {};
    }

    ParseResult
    TestMarketDataParser::parseBookUpdate(const std::string_view data,
                                          BookUpdate& bookUpdate)
    {
        if (data.empty())
            return ParseResult::EmptyMessage;

        bookUpdate.clear();
        std::vector<std::string_view> fields;
        for (size_t start = 0, end = 0; end <= data.size(); ++end) {
            if (end == data.size() || data[end] == ',') {
                fields.push_back(data.substr(start, end - start));
                start = end + 1;
            }
        }

        if (fields.size() < 4) {
            return ParseResult::InvalidMessage;
        }

        if (!parseNumber(fields[0], bookUpdate.instrument))
            return ParseResult::InvalidInstrument;
        if (!parseNumber(fields[1], bookUpdate.sequenceRange.first))
            return ParseResult::InvalidSequence;
        if (!parseNumber(fields[2], bookUpdate.sequenceRange.last))
            return ParseResult::InvalidSequence;

        int64_t value { 0 };
        if (!parseNumber(fields[3], value))
            return ParseResult::InvalidTimestamp;
        bookUpdate.exchangeTimestamp = static_cast<decltype(bookUpdate.exchangeTimestamp)>(value);

        for (uint32_t idx = 4; (idx + 3) <= fields.size();)
        {
            auto&[side, price, quantity] = bookUpdate.updates.emplace_back();
            if (fields[idx] == "Buy") {
                side = Side::Buy;
            } else if (fields[idx] == "Sell") {
                side = Side::Sell;
            } else {
                return ParseResult::InvalidSide;
            }

            ++idx;

            if (!parseNumber(fields[idx++], value))
                return ParseResult::InvalidPrice;
            price = static_cast<decltype(price)>(value);

            if (!parseNumber(fields[idx++], value))
                return ParseResult::InvalidQuantity;
            quantity = static_cast<decltype(quantity)>(value);
        }

        return ParseResult::Success;
    }

}
