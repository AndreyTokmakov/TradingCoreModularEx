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

namespace trading::execution
{
    ExecutionReportModule::ExecutionReportModule(const config::Config& config,
                              concurrency::Queue<ExecutionWorkItem>& executionQueue,
                              const exchanges::IExchangeFactory& exchangeFactory,
                              const common::RuntimeContext& runtimeContext) noexcept:
    executionQueue {
        executionQueue
    },
    executionReportSource {
        exchangeFactory.createExecutionReportSource(config, executionQueue, runtimeContext)
    } {
        /** **/
    }

    void ExecutionReportModule::run() const
    {
        executionReportSource->start();
    }
}
