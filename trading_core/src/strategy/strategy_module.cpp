/**============================================================================
Name        : strategy_module.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Executes strategy processing on the strategy thread.
============================================================================**/

#include "strategy_module.hpp"

namespace trading::strategy
{
    StrategyModule::StrategyModule(const config::StrategyConfig& strategyConfig,
                                   concurrency::Queue<market_data::MarketEvent>& strategyEventQueue,
                                   concurrency::Queue<execution::ExecutionWorkItem>& executionQueue,
                                   const common::RuntimeContext&):
        strategy {
            strategyConfig.thresholdNumerator,
            strategyConfig.thresholdDenominator
        },
        executor {
            executionQueue,
            strategyConfig.orderQuantity
        },
        marketEventQueue { strategyEventQueue },
        executionQueue { executionQueue }
    {
    }

    void StrategyModule::run() const
    {
        market_data::MarketEvent event {};
        while (marketEventQueue.waitPop(event))
        {
            const Signal signal = strategy.evaluate(event);
            executor.execute(signal, event);
            /*
            const StrategyExecutionResult result = executor.execute(signal, event);
            if (!result) {
                // TODO: log / metrics / risk event.
                continue;
            }
            if (!result->has_value())
                continue;
            // TODO: order-created event / metrics.
            */
        }
    }
}