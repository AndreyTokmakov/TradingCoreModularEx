/**============================================================================
Name        : order_manager_module.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Processes order requests and execution reports on the execution thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_MODULE_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_MODULE_HPP

#include "execution/model/execution_work_item.hpp"
#include "order_manager.hpp"
#include "common/common.hpp"
#include "recording/recording_event.hpp"
#include "config/config.hpp"
#include "exchanges/exchange_factory.hpp"
#include "risk/risk_manager.hpp"
#include "metrics/metrics_collector.hpp"
#include "logging/logger.hpp"

namespace trading::execution
{
    class OrderManagerModule final: public common::Worker<OrderManagerModule>
    {
    public:
        OrderManagerModule(const config::Config& config,
                           common::Queue<ExecutionWorkItem>& executionQueue,
                           common::Queue<recording::RecordingEvent>& recordingQueue,
                           const exchanges::IExchangeFactory& exchangeFactory) noexcept;

        void run();

    private:

        void process(const OrderRequest& request);
        void process(const ExecutionReport& report);

    private:

        position::PositionManager positionManager;
        risk::RiskManager riskManager;

        common::Queue<ExecutionWorkItem>& executionQueue;
        common::Queue<recording::RecordingEvent>& recordingQueue;

        std::unique_ptr<IOrderGateway> orderGateway;
        OrderManager orderManager;

        std::shared_ptr<logging::ILogger> logger;
        static inline thread_local metrics::Metrics& metrics  = metrics::MetricsCollector::getCollector().getThreadLocalMetrics();
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_MODULE_HPP