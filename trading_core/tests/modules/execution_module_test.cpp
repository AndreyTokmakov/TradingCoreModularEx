/**============================================================================
Name        : execution_module_test.cpp
Created on  : 10.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_module_test.cpp
============================================================================**/


#include "execution_module.hpp"
#include "condition_variable_queue.hpp"
#include "test_support/testing.hpp"
#include "test_support/test_exchange_factory.hpp"
#include "test_support/test_execution_gateway.hpp"

#include <iostream>
#include <variant>

using trading::InstrumentId;
using trading::OrderStatus;
using trading::OrderType;
using trading::Price;
using trading::Quantity;
using trading::Side;
using trading::concurrency::ConditionVariableQueue;
using trading::execution::ExecutionModule;
using trading::execution::ExecutionReport;
using trading::execution::ExecutionWorkItem;
using trading::execution::OrderRequest;
using trading::recording::RecordingEvent;

namespace
{
    using testing::Assert;
    using TestExecutionGateway = trading::testing::TestExecutionGateway;

    constexpr InstrumentId Instrument { 42 };

    constexpr Price OrderPrice { 6'500'000'000'000 };
    constexpr Quantity OrderQuantity { 100'000'000 };

    constexpr trading::OrderId ClientOrderId { 1 };
    constexpr trading::ExchangeOrderId ExchangeOrderId { 12345 };

    [[nodiscard]]
    trading::config::Config createConfig()
    {
        return trading::config::Config {};
    }

    [[nodiscard]]
    OrderRequest createBuyOrderRequest()
    {
        return OrderRequest {
            .instrument = Instrument,
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = OrderPrice,
            .quantity = OrderQuantity
        };
    }

    [[nodiscard]]
    ExecutionReport createTradeReport()
    {
        return ExecutionReport {
            .clientOrderId = ClientOrderId,
            .exchangeOrderId = ExchangeOrderId,
            .instrument = Instrument,
            .side = Side::Buy,
            .execType = trading::ExecType::Trade,
            .status = OrderStatus::Filled,
            .price = OrderPrice,
            .quantity = OrderQuantity,
            .filledQuantity = OrderQuantity
        };
    }


    class TestExecutionExchangeFactory final : public trading::exchanges::IExchangeFactory
    {
    public:
        explicit TestExecutionExchangeFactory(std::unique_ptr<TestExecutionGateway> gateway) noexcept:
                                               gateway { std::move(gateway) }
        {
        }

        [[nodiscard]]
        std::unique_ptr<trading::execution::IExecutionGateway>
        createExecutionGateway(const trading::config::Config&) const noexcept override
        {
            return std::move(gateway);
        }

        [[nodiscard]]
        std::unique_ptr<trading::execution::IExecutionReportSource>
        createExecutionReportSource(const trading::config::Config&,
                                    trading::concurrency::Queue<ExecutionWorkItem>&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<trading::market_data::IMarketDataParser>
        createMarketDataParser(const trading::config::Config&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<trading::market_data::IMarketDataSource>
        createMarketDataSource(const trading::config::Config&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<trading::market_data::ISnapshotProvider>
        createSnapshotProvider(const trading::config::Config&) const noexcept override
        {
            return nullptr;
        }

    private:
        mutable std::unique_ptr<TestExecutionGateway> gateway;
    };

    /**
     * Tests that a valid OrderRequest is accepted by the ExecutionModule,
     * converted into an Order by OrderManager and sent to the execution gateway.
     *
     * Expected result:
     * - exactly one order is sent to the gateway;
     * - all order fields are copied from the OrderRequest;
     * - the new order has a zero filled quantity;
     * - the new order has OrderStatus::New.
     */
    void testOrderRequestCreatesAndSendsOrder()
    {
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto gateway = std::make_unique<TestExecutionGateway>();
        TestExecutionGateway* gatewayPtr = gateway.get();

        TestExecutionExchangeFactory exchangeFactory { std::move(gateway) };
        ExecutionModule module { createConfig(), executionQueue, recordingQueue, exchangeFactory };

        executionQueue.push(createBuyOrderRequest());
        executionQueue.close();

        module.run();
        Assert(gatewayPtr->sendOrdersCount() == 1, "OrderRequest must be sent to execution gateway");

        const auto& order = gatewayPtr->lastSendOrder();

        Assert(order.clientOrderId == ClientOrderId, "invalid client order id");
        Assert(order.instrument == Instrument, "invalid order instrument");
        Assert(order.side == Side::Buy, "invalid order side");
        Assert(order.type == OrderType::Limit, "invalid order type");
        Assert(order.price == OrderPrice, "invalid order price");
        Assert(order.quantity == OrderQuantity, "invalid order quantity");
        Assert(order.filledQuantity.isZero(), "new order must have zero filled quantity");
        Assert(order.status == OrderStatus::New, "new order must have New status");
    }


    /**
     * Tests that every OrderRequest processed by the ExecutionModule is forwarded
     * to the recording queue as a RecordingEvent.
     *
     * Expected result:
     * - exactly one recording event is produced;
     * - the event contains an OrderRequest;
     * - all fields of the recorded request match the original request.
     */
    void testOrderRequestIsRecorded()
    {
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto gateway = std::make_unique<TestExecutionGateway>();
        TestExecutionExchangeFactory exchangeFactory { std::move(gateway) };
        ExecutionModule module { createConfig(), executionQueue, recordingQueue, exchangeFactory };

        const OrderRequest request = createBuyOrderRequest();

        executionQueue.push(request);
        executionQueue.close();

        module.run();

        RecordingEvent event {};

        Assert(recordingQueue.tryPop(event), "OrderRequest must be recorded");
        Assert(std::holds_alternative<OrderRequest>(event),"recording event must contain OrderRequest");

        const OrderRequest& recordedRequest = std::get<OrderRequest>(event);

        Assert(recordedRequest.instrument == request.instrument, "invalid recorded instrument");
        Assert(recordedRequest.side == request.side, "invalid recorded side");
        Assert(recordedRequest.type == request.type, "invalid recorded type");
        Assert(recordedRequest.price == request.price, "invalid recorded price");
        Assert(recordedRequest.quantity == request.quantity, "invalid recorded quantity");
        Assert(recordingQueue.empty(), "only one recording event must be produced");
    }

    /**
     * Tests that an invalid OrderRequest rejected by OrderManager/RiskManager
     * is not sent to the execution gateway.
     *
     * Expected result:
     * - no order is sent to the gateway;
     * - the original OrderRequest is still recorded.
     */
    void testRiskRejectedOrderIsNotSent()
    {
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto gateway = std::make_unique<TestExecutionGateway>();
        TestExecutionGateway* gatewayPtr = gateway.get();

        TestExecutionExchangeFactory exchangeFactory { std::move(gateway) };
        ExecutionModule module { createConfig(), executionQueue, recordingQueue, exchangeFactory};

        OrderRequest request = createBuyOrderRequest();
        request.quantity = Quantity { 0 };

        executionQueue.push(request);
        executionQueue.close();

        module.run();

        Assert(gatewayPtr->sendOrdersCount() == 0,"risk-rejected order must not be sent to execution gateway");

        RecordingEvent event {};

        Assert(recordingQueue.tryPop(event), "rejected OrderRequest must still be recorded");
        Assert(std::holds_alternative<OrderRequest>(event),"recording event must contain OrderRequest");
    }


    /**
     * Tests that an ExecutionReport received after creating an order is passed
     * to OrderManager and updates the corresponding order.
     *
     * Expected result:
     * - the OrderRequest is processed first;
     * - the ExecutionReport is processed second;
     * - both work items are successfully consumed;
     * - both corresponding recording events are produced.
     */
    void testExecutionReportUpdatesOrder()
    {
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto gateway = std::make_unique<TestExecutionGateway>();
        TestExecutionExchangeFactory exchangeFactory { std::move(gateway) };
        ExecutionModule module { createConfig(), executionQueue, recordingQueue, exchangeFactory};

        executionQueue.push(createBuyOrderRequest());
        executionQueue.push(createTradeReport());
        executionQueue.close();

        module.run();

        Assert(executionQueue.empty(), "all execution work items must be consumed");

        RecordingEvent firstEvent {};
        RecordingEvent secondEvent {};

        Assert(recordingQueue.tryPop(firstEvent), "OrderRequest must be recorded");
        Assert(recordingQueue.tryPop(secondEvent), "ExecutionReport must be recorded");
        Assert(std::holds_alternative<OrderRequest>(firstEvent),"first recording event must contain OrderRequest");
        Assert(std::holds_alternative<ExecutionReport>(secondEvent),"second recording event must contain ExecutionReport");
    }

    /**
     * Tests that an ExecutionReport is forwarded to the recording queue before
     * being processed by OrderManager.
     *
     * Expected result:
     * - exactly one recording event is produced;
     * - the event contains an ExecutionReport;
     * - all report fields are preserved.
     */
    void testExecutionReportIsRecorded()
    {
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto gateway = std::make_unique<TestExecutionGateway>();
        TestExecutionExchangeFactory exchangeFactory { std::move(gateway) };
        ExecutionModule module { createConfig(), executionQueue, recordingQueue, exchangeFactory};

        executionQueue.push(createTradeReport());
        executionQueue.close();

        module.run();

        RecordingEvent event {};

        Assert(recordingQueue.tryPop(event), "ExecutionReport must be recorded");
        Assert(std::holds_alternative<ExecutionReport>(event),"recording event must contain ExecutionReport");

        const ExecutionReport& report = std::get<ExecutionReport>(event);

        Assert(report.clientOrderId == ClientOrderId, "invalid recorded client order id");
        Assert(report.exchangeOrderId == ExchangeOrderId, "invalid recorded exchange order id");
        Assert(report.instrument == Instrument, "invalid recorded instrument");
        Assert(report.side == Side::Buy, "invalid recorded side");
        Assert(report.execType == trading::ExecType::Trade, "invalid recorded exec type");
        Assert(report.status == OrderStatus::Filled, "invalid recorded status");
        Assert(report.price == OrderPrice, "invalid recorded price");
        Assert(report.quantity == OrderQuantity, "invalid recorded quantity");
        Assert(report.filledQuantity == OrderQuantity, "invalid recorded filled quantity");
    }

    /**
     * Tests that a Trade ExecutionReport is accepted by ExecutionModule and
     * forwarded into the position-processing path of OrderManager.
     *
     * Expected result:
     * - the OrderRequest is processed;
     * - the Trade ExecutionReport is processed;
     * - both corresponding recording events are produced;
     * - module processing completes without an error.
     */
    void testTradeUpdatesPosition()
    {
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto gateway = std::make_unique<TestExecutionGateway>();
        TestExecutionExchangeFactory exchangeFactory { std::move(gateway) };
        ExecutionModule module { createConfig(), executionQueue, recordingQueue, exchangeFactory};

        executionQueue.push(createBuyOrderRequest());
        executionQueue.push(createTradeReport());
        executionQueue.close();

        module.run();

        RecordingEvent requestEvent {};
        RecordingEvent reportEvent {};

        Assert(recordingQueue.tryPop(requestEvent), "OrderRequest must be recorded");
        Assert(recordingQueue.tryPop(reportEvent), "ExecutionReport must be recorded");
    }

    /**
     * Tests that an ExecutionReport referencing an unknown client order ID
     * does not create or send any new order through the execution gateway.
     *
     * Expected result:
     * - no order is sent to the gateway;
     * - the unknown ExecutionReport is still recorded;
     * - the module continues processing normally.
     */
    void testUnknownOrderExecutionDoesNotCreateOrder()
    {
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto gateway = std::make_unique<trading::testing::TestExecutionGateway>();
        auto* gatewayPtr = gateway.get();

        TestExecutionExchangeFactory exchangeFactory { std::move(gateway) };
        ExecutionModule module { createConfig(), executionQueue, recordingQueue, exchangeFactory};

        ExecutionReport report = createTradeReport();
        report.clientOrderId = 999;

        executionQueue.push(report);
        executionQueue.close();

        module.run();

        Assert(gatewayPtr->sendOrdersCount() == 0,"execution report for unknown order must not send an order");

        RecordingEvent event {};

        Assert(recordingQueue.tryPop(event), "unknown execution report must still be recorded");
        Assert(std::holds_alternative<ExecutionReport>(event),"recording event must contain ExecutionReport");
    }

    /**
     * Tests that ExecutionModule processes multiple OrderRequests in queue order
     * and creates a separate exchange order for each request.
     *
     * Expected result:
     * - two orders are sent to the gateway;
     * - the first order receives clientOrderId 1;
     * - the second order receives clientOrderId 2;
     * - instrument and price values are preserved for both orders.
     */
    void testMultipleWorkItemsAreProcessedInOrder()
    {
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto gateway = std::make_unique<TestExecutionGateway>();
        TestExecutionGateway* gatewayPtr = gateway.get();

        TestExecutionExchangeFactory exchangeFactory { std::move(gateway) };
        ExecutionModule module { createConfig(), executionQueue, recordingQueue, exchangeFactory};

        const OrderRequest firstRequest = createBuyOrderRequest();

        OrderRequest secondRequest = createBuyOrderRequest();
        secondRequest.instrument = InstrumentId { 43 };
        secondRequest.price = Price { 6'600'000'000'000 };

        executionQueue.push(firstRequest);
        executionQueue.push(secondRequest);
        executionQueue.close();

        module.run();

        Assert(gatewayPtr->sendOrdersCount() == 2,"two OrderRequests must produce two gateway orders");

        const auto& firstOrder = gatewayPtr->getOrderByIndex(0);
        const auto& secondOrder = gatewayPtr->getOrderByIndex(1);


        Assert(firstOrder.clientOrderId == 1, "first order must have id 1");
        Assert(secondOrder.clientOrderId == 2, "second order must have id 2");
        Assert(firstOrder.instrument == Instrument, "invalid first order instrument");
        Assert(secondOrder.instrument == InstrumentId { 43 },"invalid second order instrument");
        Assert(recordingQueue.empty() == false,"recording queue must contain processed work items");
    }
}

void execution_module_test()
{
    testOrderRequestCreatesAndSendsOrder();
    testOrderRequestIsRecorded();
    testRiskRejectedOrderIsNotSent();
    testExecutionReportUpdatesOrder();
    testExecutionReportIsRecorded();
    testTradeUpdatesPosition();
    testUnknownOrderExecutionDoesNotCreateOrder();
    testMultipleWorkItemsAreProcessedInOrder();

    std::cout << "All ExecutionModule tests: OK\n";
}