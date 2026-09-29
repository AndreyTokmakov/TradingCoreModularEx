/**============================================================================
Name        : book_level_update.hpp
Created on  : 28.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_level_update.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_BOOK_LEVEL_UPDATE_HPP
#define TRADINGCOREMODULAREX_BOOK_LEVEL_UPDATE_HPP

#include "book_level.hpp"
#include "core/types.hpp"

namespace trading::market_data
{
    struct PriceLevelUpdate
    {
        Side side { Side::Buy };
        Price price {};
        Quantity quantity {};
    };
}

#endif //TRADINGCOREMODULAREX_BOOK_LEVEL_UPDATE_HPP
