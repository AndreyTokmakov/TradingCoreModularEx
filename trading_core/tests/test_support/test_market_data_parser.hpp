/**============================================================================
Name        : test_market_data_parser.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_market_data_parser.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TEST_MARKET_DATA_PARSER_HPP
#define FINANCETECHNOLOGYPROJECTS_TEST_MARKET_DATA_PARSER_HPP

#include "market_data_parser.hpp"

#include <vector>

namespace trading::testing
{
    class TestMarketDataParser final : public market_data::IMarketDataParser
    {
    public:
        [[nodiscard]]
        market_data::ParseResult parse(std::string_view message,
                                       market_data::BookUpdates& bookUpdates) const override;
    private:
        static market_data::ParseResult parseBookUpdate(std::string_view data,
                                                        market_data::BookUpdate& bookUpdate);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TEST_MARKET_DATA_PARSER_HPP
