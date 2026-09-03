/**============================================================================
Name        : market_event_dispatcher.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Dispatches MarketEvents to independent pipeline consumers.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_DISPATCHER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_DISPATCHER_HPP

#include "model/market_event.hpp"
#include "queue.hpp"
#include "recording_event.hpp"

namespace trading::market_data
{
    class MarketEventDispatcher final
    {
    public:
        MarketEventDispatcher(concurrency::Queue<MarketEvent>& strategyQueue,
                              concurrency::Queue<recording::RecordingEvent>& recordingEventQueue) noexcept;

        void onMarketEvent(const MarketEvent& event);

    private:
        concurrency::Queue<MarketEvent>& strategyQueue;
        concurrency::Queue<recording::RecordingEvent>& recordingEventQueue;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_DISPATCHER_HPP