/**============================================================================
Name        : trading_integration_test.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Trading pipeline integration test.
============================================================================**/

#include "test_support/testing.hpp"
#include "test_support/test_execution_gateway.hpp"
#include "test_support/test_market_data_parser.hpp"
#include "test_support/test_market_data_source.hpp"
#include "test_support/trading_test_environment.hpp"

namespace
{
    using trading::market_data::OrderBook;
    using trading::market_data::BookUpdate;

    using trading::Price;
    using trading::Quantity;
    using testing::Assert;
    using trading::Timestamp;
    using trading::SequenceNumber;
    using trading::Side;
    using trading::OrderType;
    using trading::InstrumentId;
    using trading::OrderStatus;

    using trading::testing::TradingTestEnvironment;
    using trading::testing::TradingTestConfiguration;

    void applyInitialSnapshot(const TradingTestEnvironment& environment)
    {
        const trading::market_data::Snapshot snapshot {
            .instrument = InstrumentId { 1 },
            .sequence = SequenceNumber { 0 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {{ Price { 60000 }, Quantity { 100 } }},
            .asks = {{ Price { 60001 }, Quantity { 1 } }}
        };

        const bool applied = environment.bookBuilder.applySnapshot(snapshot);

        Assert(applied,"Initial order book snapshot must be applied");
        Assert(environment.orderBook.isValid(), "OrderBook must be valid after snapshot");
        Assert(environment.recorder.marketEventCount() == 0, "Snapshot must not produce MarketEvent");
    }
}

void trading_integration_test()
{
    const auto marketDataSource { std::make_shared<trading::testing::TestMarketDataSource>() };
    const auto marketDataParser { std::make_shared<trading::testing::TestMarketDataParser>() };
    const auto executionGateway { std::make_shared<trading::testing::TestExecutionGateway>() };
    constexpr auto config = TradingTestConfiguration {
        .instrument = InstrumentId { 1 },
        .orderQuantity = Quantity { 1 },
        .thresholdNumerator = 7,
        .thresholdDenominator = 10
    };

    const TradingTestEnvironment environment { marketDataSource, marketDataParser, executionGateway, config };
    applyInitialSnapshot(environment);

    Assert(environment.recorder.marketEventCount() == 0, "No MarketEvent must exist after snapshot");
    Assert(executionGateway->orderCount() == 0, "No order must be submitted after snapshot");

    marketDataSource->addTestMarketData({"1,1,1640995200000,Buy,98765,100,1000"});
    marketDataSource->start();

    Assert(environment.recorder.marketEventCount() == 1, "One MarketEvent must be produced");
    Assert(executionGateway->hasOrder(), "Strategy must eventually send an order to execution gateway");
    Assert(executionGateway->orderCount() == 1, "One order must be submitted");

    const trading::execution::Order& order = executionGateway->lastOrder();

    // TODO:
    //   - почему OrderType::Limit
    //   - почему Price  60001

    Assert(order.clientOrderId == trading::OrderId { 1 },"Last order must have client order ID 1");
    Assert(order.instrument == environment.instrument,"Order instrument must match configured instrument");
    Assert(order.side == Side::Buy,"Strong bid imbalance must produce Buy order");
    Assert(order.type == OrderType::Limit,"StrategyExecutor must create a Limit order");
    Assert(order.quantity == environment.orderQuantity,"Order quantity must match StrategyExecutor configuration");
    Assert(order.price == Price { 60001 },"Buy order must use best ask price");
    Assert(order.status == OrderStatus::New,"Newly submitted order must have New status");

    const auto& recorder = environment.recorder;
    Assert(recorder.marketEventCount() == 1, "One MarketEvents must be recorded");
}