/**============================================================================
Name        : execution_module.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Sends orders to the exchange on the execution thread.
============================================================================**/

#include "config_utils.hpp"
#include "execution_module.hpp"
#include "binance_execution_gateway.hpp"
#include "binance_execution_report_source.hpp"


namespace trading::execution
{
    namespace binance = exchanges::binance;

    ExecutionModule::ExecutionModule(const config::Config& config,
                                     concurrency::Queue<ExecutionWorkItem>& executionQueue,
                                     concurrency::Queue<recording::RecordingEvent>& recordingQueue,
                                     const common::RuntimeContext& runtimeContext) noexcept:
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
            std::make_unique<binance::BinanceExecutionGateway>(findExchange(config, "binance").executionEndpoint)
        },
        executionReportSource {
            std::make_unique<binance::BinanceExecutionReportSource>(findExchange(config, "binance").executionEndpoint, executionQueue)
        },
        orderManager {
            *riskManager, positionManager, *executionGateway
        },
        logger {
            runtimeContext.logger
        },
        metricsCollector {
            runtimeContext.metricsCollector
        }
    {
        /** TODO **/
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

    void ExecutionModule::process(const OrderRequest& request)
    {
        logger->info("{} [{}]", __PRETTY_FUNCTION__, __LINE__);
        metrics->increment<metrics::MetricType::OrderRequests>();
        recordingQueue.push(request);

        [[maybe_unused]]
        const OrderCreationResult result = orderManager.createOrder(request);

        logger->info("{} [{}]", __PRETTY_FUNCTION__, __LINE__);
        // TODO: Handle order creation errors: logging / metrics / risk event.
    }

    void ExecutionModule::process(const ExecutionReport& report)
    {
        metrics->increment<metrics::MetricType::ExecutionReport>();
        recordingQueue.push(report);

        [[maybe_unused]]
        const bool processed = orderManager.applyExecution(report);
        // TODO: Handle unknown orders or invalid execution reports.
    }
}