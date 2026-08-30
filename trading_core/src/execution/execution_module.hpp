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

#include "execution_work_item.hpp"
#include "order_manager.hpp"
#include "queue.hpp"
#include "recorder.hpp"
#include "runtime_context.hpp"
#include "worker.hpp"

namespace trading::execution
{
    class ExecutionModule final: public common::Worker<ExecutionModule>
    {
    public:
        ExecutionModule(concurrency::Queue<ExecutionWorkItem>& executionQueue,
                        OrderManager& orderManager,
                        recording::IRecorder& recorder,
                        const common::RuntimeContext& runtimeContext) noexcept;

        void run();

    private:

        void process(const OrderRequest& request) const;
        void process(const ExecutionReport& report) const;

        concurrency::Queue<ExecutionWorkItem>& executionQueue;
        OrderManager& orderManager;
        recording::IRecorder& recorder;

        logging::ILogger& logger;
        metrics::MetricsCollector& metricsCollector;
        metrics::Metrics* metrics { nullptr };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_MODULE_HPP