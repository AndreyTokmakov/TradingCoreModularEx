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
#include "config.hpp"
#include "imbalance_strategy.hpp"
#include "queue.hpp"
#include "worker.hpp"
#include "strategy.hpp"
#include "strategy_executor.hpp"


namespace trading::strategy
{
    class StrategyModule final: public common::Worker<StrategyModule>
    {
    public:
        StrategyModule(const config::StrategyConfig& strategyConfig,
                       concurrency::Queue<market_data::MarketEvent>& strategyEventQueue,
                       concurrency::Queue<execution::ExecutionWorkItem>& executionQueue);

        void run() const;

    private:

        ImbalanceStrategy strategy;
        StrategyExecutor executor;

        concurrency::Queue<market_data::MarketEvent>& marketEventQueue;
        concurrency::Queue<execution::ExecutionWorkItem>& executionQueue;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_STRATEGY_MODULE_HPP