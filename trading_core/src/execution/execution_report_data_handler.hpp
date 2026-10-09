/**============================================================================
Name        : execution_report_data_handler.hpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_data_handler.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_EXECUTION_REPORT_DATA_HANDLER_HPP
#define TRADINGCOREMODULAREX_EXECUTION_REPORT_DATA_HANDLER_HPP

#include "execution/interfaces/execution_report_handler.hpp"
#include "execution/model/execution_work_item.hpp"
#include "common/queue.hpp"
#include "metrics/metrics_collector.hpp"
#include "logging/logger.hpp"

namespace trading::execution
{
    class ExecutionReportDataHandler: public IExecutionReportHandler
    {
    public:
        explicit ExecutionReportDataHandler(concurrency::Queue<ExecutionWorkItem>& executionQueue);

        void onExecutionReport(std::string_view message) override;

    private:

        void publish(const ExecutionReport& report) const;

    private:
        concurrency::Queue<ExecutionWorkItem>& executionQueue;

        std::shared_ptr<logging::ILogger> logger;
        static inline thread_local metrics::Metrics& metrics  = metrics::MetricsCollector::getCollector().getThreadLocalMetrics();
    };
}


#endif //TRADINGCOREMODULAREX_EXECUTION_REPORT_DATA_HANDLER_HPP
