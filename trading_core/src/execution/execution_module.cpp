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
    using LoggerFactory = logging::LoggerFactory;

    ExecutionModule::ExecutionModule(const config::Config& config,
                                     concurrency::Queue<ExecutionWorkItem>& executionQueue,
                                     concurrency::Queue<recording::RecordingEvent>& recordingQueue,
                                     const exchanges::IExchangeFactory& exchangeFactory) noexcept:
        positionManager{},
        riskManager {},
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
            riskManager, positionManager, *executionGateway
        },
        logger { LoggerFactory::getLogger() }
    {
        /** **/
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
        metrics.increment<metrics::MetricType::OrderRequests>();
        recordingQueue.push(request);

        const OrderCreationResult result = orderManager.createOrder(request);
        if (!result.has_value()) {
            // TODO: Handle order creation errors: logging / metrics / risk event.
        }

        metrics.increment<metrics::MetricType::OrdersSubmitted>();
    }

    void ExecutionModule::process(const ExecutionReport& report)
    {
        metrics.increment<metrics::MetricType::ExecutionReports>();
        recordingQueue.push(report);

        [[maybe_unused]]
        const bool processed = orderManager.applyExecution(report);
        if (!processed) {
            metrics.increment<metrics::MetricType::ExecutionErrors>();
            // TODO: Handle unknown orders or invalid execution reports.
        }
    }
}