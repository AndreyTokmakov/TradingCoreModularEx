/**============================================================================
Name        : trade_module.cpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trade_module.cpp
============================================================================**/

#include "trade_module.hpp"

namespace trading::trade
{
    TradeModule::TradeModule(const config::Config& config,
                             concurrency::Queue<Trade>& tradeQueue) noexcept :
        tradeQueue { tradeQueue },
        tradeProcessor { config.instrument }
    {
    }

    void TradeModule::run()
    {
        Trade trade;

        while (tradeQueue.waitPop(trade)) {
            tradeProcessor.onTrade(trade);
        }
    }

    const TradeProcessor& TradeModule::processor() const noexcept
    {
        return tradeProcessor;
    }
}