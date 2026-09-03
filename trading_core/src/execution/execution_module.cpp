/**============================================================================
Name        : execution_module.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Sends orders to the exchange on the execution thread.
============================================================================**/

#include "execution_module.hpp"

namespace trading::execution
{
    ExecutionModule::ExecutionModule(const config::Config& config,
                                     concurrency::Queue<ExecutionWorkItem>& executionQueue,
                                     concurrency::Queue<recording::RecordingEvent>& recordingQueue,
                                     const exchanges::IExchangeFactory& exchangeFactory) noexcept:
        positionManager{},
        riskManager {
            std::make_unique<risk::RiskManager>(config.riskLimits)
        },
        executionQueue {
            executionQueue
        },
        recordingQueue {
            recordingQueue
        },
        executionGateway {
            exchangeFactory.createExecutionGateway(config)
        },
        orderManager {
            *riskManager, positionManager, *executionGateway
        }
    {
        /** TODO **/
    }

    void ExecutionModule::run()
    {
        ExecutionWorkItem workItem;
        while (executionQueue.waitPop(workItem))
        {
            std::visit([this](const auto& item) {
                process(item);
            },workItem);
        }
    }

    void ExecutionModule::process(const OrderRequest& request)
    {
        // logger->info("{} [{}]", __PRETTY_FUNCTION__, __LINE__);
        metrics.increment<metrics::MetricType::OrderRequests>();
        recordingQueue.push(request);

        [[maybe_unused]]
        const OrderCreationResult result = orderManager.createOrder(request);

        // logger->info("{} [{}]", __PRETTY_FUNCTION__, __LINE__);
        // TODO: Handle order creation errors: logging / metrics / risk event.
    }

    void ExecutionModule::process(const ExecutionReport& report)
    {
        metrics.increment<metrics::MetricType::ExecutionReports>();
        recordingQueue.push(report);

        [[maybe_unused]]
        const bool processed = orderManager.applyExecution(report);
        // TODO: Handle unknown orders or invalid execution reports.
    }
}