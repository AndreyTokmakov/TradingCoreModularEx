/**============================================================================
Name        : debug_helpers.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Testing and Debuging
============================================================================**/

#include <iostream>

#include "debug_helpers.hpp"
#include "model/market_event.hpp"

namespace trading::testing
{
    void printMarketEvent(const trading::market_data::MarketEvent &event)
    {
        std::cout << "MarketEvent {"
            << "\n\t instrument       : " << event.instrument
            << "\n\t sequence         : " << event.sequence
            << "\n\t exchangeTimestamp: " << event.exchangeTimestamp.nanoseconds()
            << "\n\t receiveTimestamp : " << event.receiveTimestamp .nanoseconds()
            << "\n\t bestBid          : " << event.bestBid.raw()
            << "\n\t bestBidQuantity  : " << event.bestBidQuantity.raw()
            << "\n\t bestAsk          : " << event.bestAsk.raw()
            << "\n\t bestAskQuantity  : " << event.bestAskQuantity.raw()
            << "\n}" << std::endl;

    }
}

