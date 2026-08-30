/**============================================================================
Name        : execution_module.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Sends orders to the exchange on the execution thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_MODULE_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_MODULE_HPP

#include "model/execution_work_item.hpp"
#include "execution_report_source.hpp"
#include "order_manager.hpp"
#include "queue.hpp"
#include "recording_event.hpp"
#include "runtime_context.hpp"
#include "worker.hpp"
#include "config.hpp"

namespace trading::execution
{
    class ExecutionModule final: public common::Worker<ExecutionModule>
    {
    public:
        ExecutionModule(const config::Config& config,
                        concurrency::Queue<ExecutionWorkItem>& executionQueue,
                        concurrency::Queue<recording::RecordingEvent>& recordingQueue,
                        const common::RuntimeContext& runtimeContext) noexcept;

        void run();

    private:

        void process(const OrderRequest& request);
        void process(const ExecutionReport& report);

    private:

        position::PositionManager positionManager;
        std::unique_ptr<risk::IRiskManager> riskManager;

        concurrency::Queue<ExecutionWorkItem>& executionQueue;
        concurrency::Queue<recording::RecordingEvent>& recordingQueue;

        std::unique_ptr<IExecutionGateway> executionGateway;
        OrderManager orderManager;

        std::shared_ptr<logging::ILogger> logger;
        metrics::MetricsCollector& metricsCollector; // TODO: Refactor: Не нужно навенрное MetricsCollector или Metrics
        metrics::Metrics* metrics { nullptr };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_MODULE_HPP