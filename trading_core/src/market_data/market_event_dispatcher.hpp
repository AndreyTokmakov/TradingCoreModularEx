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

#include "interfaces/market_event_handler.hpp"
#include "model/market_event.hpp"
#include "queue.hpp"

namespace trading::market_data
{
    class MarketEventDispatcher final : public IMarketEventHandler
    {
    public:
        MarketEventDispatcher(concurrency::Queue<MarketEvent>& strategyQueue,
                              concurrency::Queue<MarketEvent>& recordingQueue) noexcept;

        void onMarketEvent(const MarketEvent& event) override;

    private:
        concurrency::Queue<MarketEvent>& strategyQueue;
        concurrency::Queue<MarketEvent>& recordingQueue;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_DISPATCHER_HPP