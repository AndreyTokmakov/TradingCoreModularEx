/**============================================================================
Name        : test_market_data_parser.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_market_data_parser.cpp
============================================================================**/

#include "test_market_data_parser.hpp"
#include <nlohmann/json.hpp>

namespace
{
    using trading::Side;
    using trading::market_data::ParseResult;

    template<typename T>
    bool parseNumber(std::string_view str, T& out)
    {
        if (str.empty())
            return false;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
        return ec == std::errc() && ptr == str.data() + str.size();
    }

    [[maybe_unused]]
    ParseResult
    parseBookUpdate(const std::string_view data,
                    trading::market_data::BookUpdate& bookUpdate)
    {
        if (data.empty())
            return trading::market_data::ParseResult::EmptyMessage;
        std::vector<std::string_view> fields;
        for (size_t start = 0, end = 0; end <= data.size(); ++end) {
            if (end == data.size() || data[end] == ',') {
                fields.push_back(data.substr(start, end - start));
                start = end + 1;
            }
        }

        if (fields.size() != 7)
            return ParseResult::InvalidMessage;
        if (!parseNumber(fields[0], bookUpdate.instrument))
            return ParseResult::InvalidInstrument;
        if (!parseNumber(fields[1], bookUpdate.sequence))
            return ParseResult::InvalidSequence;

        int64_t value { 0 };
        if (!parseNumber(fields[2], value))
            return ParseResult::InvalidTimestamp;
        bookUpdate.exchangeTimestamp = static_cast<decltype(bookUpdate.exchangeTimestamp)>(value);

        if (fields[3] == "Buy") {
            bookUpdate.side = Side::Buy;
        } else if (fields[3] == "Sell") {
            bookUpdate.side = Side::Sell;
        } else {
            return ParseResult::InvalidSide;
        }

        if (!parseNumber(fields[4], value))
            return ParseResult::InvalidPrice;
        bookUpdate.price = static_cast<decltype(bookUpdate.price)>(value);

        if (!parseNumber(fields[5], value))
            return ParseResult::InvalidPrice;
        bookUpdate.quantity = static_cast<decltype(bookUpdate.quantity)>(value);

        return ParseResult::Success;
    }

}


namespace trading::testing::stubs
{
    market_data::ParseResult
    TestMarketDataParser::parse([[maybe_unused]] std::string_view message,
                                    market_data::BookUpdates& bookUpdates) const
    {
        bookUpdates.clear();

        const ParseResult result = parseBookUpdate(message,bookUpdates.emplace_back());
        if (ParseResult::Success != result) {
            bookUpdates.pop_back();
            return result;
        }


        return market_data::ParseResult::Success;;
    }
}