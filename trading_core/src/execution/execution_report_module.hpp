/**============================================================================
Name        : execution_report_module.hpp
Created on  : 30.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_module.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_EXECUTION_REPORT_MODULE_HPP
#define TRADINGCOREMODULAREX_EXECUTION_REPORT_MODULE_HPP

#include "model/execution_work_item.hpp"
#include "interfaces/execution_report_source.hpp"
#include "queue.hpp"
#include "runtime_context.hpp"
#include "worker.hpp"
#include "config.hpp"

namespace trading::execution
{
    class ExecutionReportModule final: public common::Worker<ExecutionReportModule>
    {
    public:
        ExecutionReportModule(const config::Config& config,
                              concurrency::Queue<ExecutionWorkItem>& executionQueue,
                              const common::RuntimeContext& runtimeContext) noexcept;
        void run() const;

    private:

        concurrency::Queue<ExecutionWorkItem>& executionQueue;
        std::unique_ptr<IExecutionReportSource> executionReportSource;
    };
}


#endif //TRADINGCOREMODULAREX_EXECUTION_REPORT_MODULE_HPP
