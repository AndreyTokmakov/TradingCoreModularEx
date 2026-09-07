/**============================================================================
Name        : execution_module.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :  Processes order requests and execution reports on the execution thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_MODULE_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_MODULE_HPP

#include "model/execution_work_item.hpp"
#include "interfaces/execution_report_source.hpp"
#include "order_manager.hpp"
#include "queue.hpp"
#include "recording_event.hpp"
#include "worker.hpp"
#include "config.hpp"
#include "exchange_factory.hpp"
#include "metrics_collector.hpp"
#include "risk_manager.hpp"

namespace trading::execution
{
    class ExecutionModule final: public common::Worker<ExecutionModule>
    {
    public:
        ExecutionModule(const config::Config& config,
                        concurrency::Queue<ExecutionWorkItem>& executionQueue,
                        concurrency::Queue<recording::RecordingEvent>& recordingQueue,
                        const exchanges::IExchangeFactory& exchangeFactory) noexcept;

        void run();

    private:

        void process(const OrderRequest& request);
        void process(const ExecutionReport& report);

    private:

        position::PositionManager positionManager;
        risk::RiskManager riskManager;

        concurrency::Queue<ExecutionWorkItem>& executionQueue;
        concurrency::Queue<recording::RecordingEvent>& recordingQueue;

        std::unique_ptr<IExecutionGateway> executionGateway;
        OrderManager orderManager;

        // std::shared_ptr<logging::ILogger> logger;
        static inline thread_local metrics::Metrics& metrics  = metrics::MetricsCollector::getCollector().getThreadLocalMetrics();
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_MODULE_HPP