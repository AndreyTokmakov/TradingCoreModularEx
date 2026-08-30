/**============================================================================
Name        : execution_module.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Sends orders to the exchange on the execution thread.
============================================================================**/

#include "execution_module.hpp"
#include "logger_factory.hpp"

namespace trading::execution
{
    ExecutionModule::ExecutionModule(concurrency::Queue<ExecutionWorkItem>& executionQueue,
                                    OrderManager& orderManager,
                                    recording::IRecorder& recorder,
                                    const common::RuntimeContext& runtimeContext) noexcept:
        executionQueue { executionQueue },
        orderManager { orderManager },
        recorder { recorder },
        logger { runtimeContext.logger },
        metricsCollector { runtimeContext.metricsCollector }
    {
        // logger = L
    }

    void ExecutionModule::run()
    {
        metrics = &metricsCollector.getThreadMetrics();
        ExecutionWorkItem workItem;
        while (executionQueue.waitPop(workItem))
        {
            std::visit([this](const auto& item) {
                process(item);
            },workItem);
        }
    }

    void ExecutionModule::process(const OrderRequest& request) const
    {
        logger.info("{} [{}]", __PRETTY_FUNCTION__, __LINE__);
        metrics->increment<metrics::MetricType::OrderRequests>();

        [[maybe_unused]]
        const OrderCreationResult result = orderManager.createOrder(request);

        logger.info("{} [{}]", __PRETTY_FUNCTION__, __LINE__);
        // TODO: Handle order creation errors: logging / metrics / risk event.
    }

    void ExecutionModule::process(const ExecutionReport& report) const
    {
        metrics->increment<metrics::MetricType::ExecutionReport>();
        recorder.record(report);

        [[maybe_unused]]
        const bool processed = orderManager.applyExecution(report);
        // TODO: Handle unknown orders or invalid execution reports.
    }
}