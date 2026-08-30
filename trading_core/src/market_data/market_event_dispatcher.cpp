/**============================================================================
Name        : market_event_dispatcher.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Dispatches MarketEvents to independent pipeline consumers.
============================================================================**/

#include "market_event_dispatcher.hpp"

namespace trading::market_data
{
    MarketEventDispatcher::MarketEventDispatcher(concurrency::Queue<MarketEvent>& strategyQueue,
                                                 concurrency::Queue<MarketEvent>& recordingQueue) noexcept:
        strategyQueue { strategyQueue },
        recordingQueue { recordingQueue }
    {
    }

    void MarketEventDispatcher::onMarketEvent(const MarketEvent& event)
    {
        strategyQueue.push(event);
        recordingQueue.push(event);
    }
}