/**============================================================================
Name        : strategy_module.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Executes strategy processing on the strategy thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_STRATEGY_MODULE_HPP
#define FINANCETECHNOLOGYPROJECTS_STRATEGY_MODULE_HPP

#include "model/market_event.hpp"
#include "queue.hpp"
#include "worker.hpp"
#include "strategy.hpp"
#include "strategy_executor.hpp"


namespace trading::strategy
{
    class StrategyModule final: public common::Worker<StrategyModule>
    {
    public:
        StrategyModule(concurrency::Queue<market_data::MarketEvent>& marketEventQueue,
                       IStrategy& strategy,
                       StrategyExecutor& executor) noexcept;

        void run() const;

    private:
        concurrency::Queue<market_data::MarketEvent>& marketEventQueue;
        IStrategy& strategy;
        StrategyExecutor& executor;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_STRATEGY_MODULE_HPP