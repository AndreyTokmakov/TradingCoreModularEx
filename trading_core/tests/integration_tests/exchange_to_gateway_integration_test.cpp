/**============================================================================
Name        : exchange_to_gateway_integration_test.cpp
Created on  : 24.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : exchange_to_gateway_integration_test.cpp
============================================================================**/

#include "order_book/book_builder_module.hpp"
#include "common/condition_variable_queue.hpp"
#include "market_data/market_data_module.hpp"
#include "strategy/strategy_module.hpp"
#include "execution/execution_module.hpp"

#include "test_support/testing.hpp"
#include "test_support/debug_helpers.hpp"
#include "test_support/test_market_data_source.hpp"
#include "test_support/trading_test_configuration.hpp"
#include "test_support/test_market_data_parser.hpp"
#include "test_support/test_execution_gateway.hpp"
#include "test_support/test_snapshot_provider.hpp"
#include "test_support/test_exchange_factory.hpp"


#include <memory>
#include <variant>

namespace
{
    using testing::Assert;
    using trading::InstrumentId;
    using trading::Price;
    using trading::Quantity;
    using trading::Side;
    using trading::order_book::BookBuilderModule;
    using namespace trading;
    using namespace trading::concurrency;
    using namespace trading::execution;
    using namespace trading::market_data;
    using namespace trading::strategy;
    using namespace trading::testing;

    constexpr Price BidPrice { 6'500'000'000'000 };
    constexpr Price AskPrice { 6'500'001'000'000 };
    constexpr Quantity BidQuantity { 120'000'000 };
    constexpr Quantity AskQuantity { 90'000'000 };

    [[nodiscard]]
    config::Config createConfig()
    {
        config::Config config {};

        config.instrument = InstrumentId { 1 };
        config.strategy.orderQuantity = Quantity { 100'000'000 };
        config.strategy.thresholdNumerator = 7;
        config.strategy.thresholdDenominator = 10;

        return config;
    }

    [[nodiscard]]
    Snapshot createSnapshot(const InstrumentId instrument,
                            const SequenceNumber sequence)
    {
        Snapshot snapshot {};
        snapshot.instrument = instrument;
        snapshot.sequence = sequence;
        snapshot.exchangeTimestamp = Timestamp { 10'000'000 };
        snapshot.bids = {{ BidPrice, BidQuantity }};
        snapshot.asks = {{ AskPrice, AskQuantity }};
        return snapshot;
    }

    [[nodiscard]]
    std::unique_ptr<TestMarketDataSource>
    createMarketDataSource(const std::vector<std::string>& messages)
    {
        auto source = std::make_unique<TestMarketDataSource>();
        source->addTestMarketData(messages);
        return source;
    }

    void Test_1()
    {
        const config::Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<Trade> tradeQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        std::unique_ptr<TestMarketDataSource> marketDataSource = createMarketDataSource({
            "1,101,101,10000001,Buy,6500000000000,1000000000"
        });

        std::unique_ptr<TestExecutionGateway> gateway = std::make_unique<TestExecutionGateway>();
        auto ptrGateway = gateway.get();

        const Snapshot snapshot = createSnapshot( InstrumentId { 1 }, SequenceNumber { 100 } );

        TestExchangeFactory exchangeFactory { TestMocks {
            .executionGateway = std::move(gateway),
            .marketDataParser = std::make_unique<TestMarketDataParser>(),
            .marketDataSource = std::move(marketDataSource),
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(snapshot)
        }};

        MarketDataModule marketDataModule { config, bookUpdateQueue, tradeQueue, exchangeFactory };
        BookBuilderModule bookBuilderModule { config,bookUpdateQueue, marketEventQueue, recordingQueue,exchangeFactory };
        StrategyModule strategyModule { config.strategy,marketEventQueue, executionQueue};
        ExecutionModule executionModule { createConfig(), executionQueue, recordingQueue, exchangeFactory };

        bookBuilderModule.start();
        strategyModule.start();
        executionModule.start();
        marketDataModule.start();



        /*
        ExecutionWorkItem workItem {};

        while (!executionQueue.tryPop(workItem)) {
            std::this_thread::yield();
        }

        Assert(std::holds_alternative<OrderRequest>(workItem),"Strategy pipeline must produce an OrderRequest");

        const OrderRequest& request = std::get<OrderRequest>(workItem);
        Assert(request.instrument == InstrumentId { 1 }, "Order instrument must match market data instrument");
        Assert(request.side == Side::Buy, "Strategy must generate Buy signal");
        Assert(request.type == OrderType::Limit, "Strategy order type must be Limit");
        Assert(request.price == Price { 0 }, "Buy order must use best ask price");
        Assert(request.quantity == Quantity { 100'000'000 },"Order quantity must match strategy configuration");
        */

        // TODO: Check gateway ????

        /*
        while (!executionQueue.) {
            std::this_thread::yield();
        }*/

        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        marketEventQueue.close();
        recordingQueue.close();
        executionQueue.close();

        executionModule.stop();
        strategyModule.stop();
        bookUpdateQueue.close();
        bookBuilderModule.stop();
        marketDataModule.stop();

        if (ptrGateway->sendOrdersCount()) {
            std::cout << ptrGateway->getOrderByIndex(0) << std::endl;
        }
    }
}

void exchange_to_gateway_integration_test()
{
    Test_1();

    std::cout << "All tests: OK\n";
}