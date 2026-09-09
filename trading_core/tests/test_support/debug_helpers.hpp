/**============================================================================
Name        : debug_helpers.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Testing and Debuging
============================================================================**/

#ifndef TRADINGCOREMODULAREX_DEBUG_HELPERS_HPP
#define TRADINGCOREMODULAREX_DEBUG_HELPERS_HPP

#include "model/market_event.hpp"

namespace trading::testing
{
    void printMarketEvent(const market_data::MarketEvent &event);
}

#endif //TRADINGCOREMODULAREX_DEBUG_HELPERS_HPP