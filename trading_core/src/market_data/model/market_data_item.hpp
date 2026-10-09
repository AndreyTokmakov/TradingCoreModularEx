/**============================================================================
Name        : market_data_item.hpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#ifndef TRADINGCOREMODULAREX_MARKET_DATA_ITEM_HPP
#define TRADINGCOREMODULAREX_MARKET_DATA_ITEM_HPP

#include "book_update.hpp"
#include "trade.hpp"

#include <variant>

namespace trading::market_data
{
    using Trades = std::vector<Trade>;
    using MarketDataItem = std::variant<std::monostate, BookUpdate, Trades>;
}

#endif //TRADINGCOREMODULAREX_MARKET_DATA_ITEM_HPP
