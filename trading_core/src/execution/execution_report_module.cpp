/**============================================================================
Name        : execution_report_module.cpp
Created on  : 30.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_module.cpp
============================================================================**/

#include "config_utils.hpp"
#include "execution_report_module.hpp"
#include "binance_execution_report_source.hpp"

namespace trading::execution
{
    namespace binance = exchanges::binance;
    using BinanceExecutionReportSource = binance::BinanceExecutionReportSource;

    ExecutionReportModule::ExecutionReportModule(const config::Config& config,
                              concurrency::Queue<ExecutionWorkItem>& executionQueue,
                              const common::RuntimeContext& ) noexcept:
    executionQueue {
        executionQueue
    },
    executionReportSource {
        std::make_unique<BinanceExecutionReportSource>(findExchange(config, "binance").executionEndpoint, executionQueue)
    }
    {

    }

    void ExecutionReportModule::run() const
    {
        executionReportSource->start();
    }
}
