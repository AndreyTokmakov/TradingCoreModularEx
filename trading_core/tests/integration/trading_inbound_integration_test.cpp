/**============================================================================
Name        : trading_inbound_integration_test.cpp
Created on  : 23.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Trading inbound path integration test.
============================================================================**/

#include "test_support/testing.hpp"
#include "test_support/test_execution_gateway.hpp"
#include "test_support/test_market_data_parser.hpp"
#include "test_support/test_market_data_source.hpp"
#include "test_support/trading_test_environment.hpp"

#include "execution_report.hpp"
#include "execution_report_handler.hpp"

namespace
{
    using trading::InstrumentId;
    using trading::OrderId;
    using trading::OrderStatus;
    using trading::OrderType;
    using trading::Price;
    using trading::Quantity;
    using trading::Side;

    using trading::execution::Order;
    using trading::execution::ExecutionReport;
    using trading::execution::OrderRequest;
    using trading::execution::OrderCreationResult;
    using trading::ExecType;

    using trading::testing::TestExecutionGateway;
    using trading::testing::TestMarketDataParser;
    using trading::testing::TestMarketDataSource;
    using trading::testing::TradingTestConfiguration;
    using trading::testing::TradingTestEnvironment;

    using testing::Assert;

    void createOrder(TradingTestEnvironment& environment)
    {
        const OrderRequest request {
            .instrument = environment.instrument,
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 60001 },
            .quantity = environment.orderQuantity
        };

        const OrderCreationResult result = environment.createOrder(request);

        Assert(result.has_value(), "OrderManager must create order successfully");
        Assert(*result == OrderId { 1 }, "First order must have client order ID 1");
    }
}

void trading_inbound_integration_test()
{
    const auto marketDataSource = std::make_shared<TestMarketDataSource>();
    const auto marketDataParser = std::make_shared<TestMarketDataParser>();
    const auto executionGateway = std::make_shared<TestExecutionGateway>();

    constexpr TradingTestConfiguration configuration {
        .instrument = InstrumentId { 1 },
        .orderQuantity = Quantity { 1 },
        .thresholdNumerator = 7,
        .thresholdDenominator = 10
    };

    TradingTestEnvironment environment { marketDataSource, marketDataParser, executionGateway, configuration };

    //   Outbound preparation
    //   We need an existing tracked Order because ExecutionReportHandler
    //   applies an execution report to an existing order.
    createOrder(environment);
    Assert(executionGateway->orderCount() == 1,"Exactly one order must be submitted to execution gateway");

    const Order* order = environment.findOrder(OrderId { 1 });

    Assert(order != nullptr,"Created order must be tracked by OrderManager");
    Assert(order->clientOrderId == OrderId { 1 },"Tracked order must have client order ID 1");
    Assert(order->exchangeOrderId == trading::ExchangeOrderId { 0 }, "Exchange order ID must be zero before execution report");
    Assert(order->status == OrderStatus::New,"Newly created order must have New status");
    Assert(order->filledQuantity == Quantity {},"New order must have zero filled quantity");

    constexpr ExecutionReport report {
        .clientOrderId = OrderId { 1 },
        .exchangeOrderId = trading::ExchangeOrderId { 50001 },
        .instrument = InstrumentId { 1 },
        .side = Side::Buy,
        .execType = ExecType::Trade,
        .status = OrderStatus::Filled,
        .price = Price { 60001 },
        .quantity = Quantity { 1 },
        .filledQuantity = Quantity { 1 }
    };

    // The only entry point into the inbound execution path.

    const bool handled = environment.executionReportHandler.onExecutionReport(report);
    Assert(handled,"ExecutionReportHandler must successfully process execution report");

    /**  Recorder **/
    Assert(environment.recorder.executionReportCount() == 1, "Exactly one execution report must be recorded");

    const auto& recordedReports = environment.recorder.executionReports();

    Assert(recordedReports.size() == 1, "Recorder must contain exactly one execution report");
    Assert(recordedReports.front().clientOrderId == OrderId { 1 }, "Recorded execution report must have correct client order ID");
    Assert(recordedReports.front().exchangeOrderId == trading::ExchangeOrderId { 50001 },"Recorded execution report must have correct exchange order ID");
    Assert(recordedReports.front().execType == ExecType::Trade, "Recorded execution report must be a Trade");

    /** OrderManager / Order **/
    order = environment.orderManager.find(OrderId { 1 });

    Assert(order != nullptr,"Order must remain tracked after execution");
    Assert(order->clientOrderId == OrderId { 1 },"Order client order ID must remain unchanged");
    Assert(order->exchangeOrderId == trading::ExchangeOrderId { 50001 }, "Order must receive exchange order ID from execution report");
    Assert(order->status == OrderStatus::Filled, "Order status must be updated from execution report");
    Assert(order->filledQuantity == Quantity { 1 }, "Order filled quantity must be updated from execution report");

    /** PositionManager **/
    const auto* position = environment.positionManager.find(configuration.instrument);
    Assert(position != nullptr,"Trade execution must create a position");

    /*
        Position-specific fields are intentionally not checked here because
        the current integration test only relies on PositionManager's public
        contract. Position field validation can be added once the Position
        model is part of this test's contract.
    */
}