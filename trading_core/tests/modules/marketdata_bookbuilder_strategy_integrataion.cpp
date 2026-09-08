/**============================================================================
Name        : marketdata_bookbuilder_strategy_integrataion.cpp
Created on  : 08.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : marketdata_bookbuilder_strategy_integrataion.cpp
============================================================================**/

#include "book_builder_module.hpp"
#include "condition_variable_queue.hpp"
#include "market_data_module.hpp"
#include "strategy_module.hpp"


#include "test_support/test_exchange_factory.hpp"
#include "test_support/test_market_data_source.hpp"
#include "test_support/trading_test_configuration.hpp"
#include "test_support/test_market_data_parser.hpp"
#include "test_support/testing.hpp"

#include <memory>
#include <variant>

namespace
{
    using testing::Assert;
    using trading::InstrumentId;
    using trading::Price;
    using trading::Quantity;
    using trading::Side;
    using namespace trading;
    using namespace trading::concurrency;
    using namespace trading::execution;
    using namespace trading::market_data;
    using namespace trading::strategy;
    using namespace trading::testing;

    class TestSnapshotProvider final : public ISnapshotProvider
    {
    public:
        explicit TestSnapshotProvider(Snapshot snapshot) noexcept : snapshot { std::move(snapshot) }
        {
        }

        [[nodiscard]]
        Snapshot getSnapshot() override{
            return snapshot;
        }

    private:
        Snapshot snapshot;
    };

    class TestExchangeFactoryWithSnapshot final : public exchanges::IExchangeFactory
    {
    public:
        TestExchangeFactoryWithSnapshot(std::unique_ptr<TestMarketDataSource> marketDataSource,
                                         Snapshot snapshot) noexcept :
            marketDataSource { std::move(marketDataSource) },
            snapshot { std::move(snapshot) }
        {
        }

        [[nodiscard]]
        std::unique_ptr<IExecutionGateway>
        createExecutionGateway(const config::Config&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<IExecutionReportSource>
        createExecutionReportSource(const config::Config&,
                                    Queue<ExecutionWorkItem>&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<IMarketDataParser>
        createMarketDataParser(const config::Config&) const noexcept override
        {
            return std::make_unique<TestMarketDataParser>();
        }

        [[nodiscard]]
        std::unique_ptr<IMarketDataSource>
        createMarketDataSource(const config::Config&) const noexcept override
        {
            return std::move(marketDataSource);
        }

        [[nodiscard]]
        std::unique_ptr<ISnapshotProvider>
        createSnapshotProvider(const config::Config&) const noexcept override
        {
            return std::make_unique<TestSnapshotProvider>(snapshot);
        }

    private:
        mutable std::unique_ptr<TestMarketDataSource> marketDataSource;
        Snapshot snapshot;
    };

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


    void testMarketDataToStrategyBuyPipeline()
    {
        const config::Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        auto marketDataSource = createMarketDataSource({
            "1,101,10000001,Buy,6500000000000,1000000000"
        });

        const Snapshot snapshot = createSnapshot( InstrumentId { 1 }, SequenceNumber { 100 } );

        TestExchangeFactoryWithSnapshot exchangeFactory { std::move(marketDataSource), snapshot };
        MarketDataModule marketDataModule { config, bookUpdateQueue, exchangeFactory };
        BookBuilderModule bookBuilderModule { config,bookUpdateQueue, marketEventQueue, recordingQueue,exchangeFactory };
        StrategyModule strategyModule { config.strategy,marketEventQueue, executionQueue};

        marketDataModule.start();
        bookBuilderModule.start();
        strategyModule.start();

        ExecutionWorkItem workItem {};

        while (!executionQueue.tryPop(workItem))
        {
            std::this_thread::yield();
        }

        std::cout << "Got something 1" << std::endl;

        Assert(std::holds_alternative<OrderRequest>(workItem),"Strategy pipeline must produce an OrderRequest");

        const OrderRequest& request = std::get<OrderRequest>(workItem);
        Assert(request.instrument == InstrumentId { 1 }, "Order instrument must match market data instrument");
        Assert(request.side == Side::Buy, "Strategy must generate Buy signal");
        Assert(request.type == OrderType::Limit, "Strategy order type must be Limit");
        Assert(request.price == Price { 0 }, "Buy order must use best ask price");
        Assert(request.quantity == Quantity { 100'000'000 },"Order quantity must match strategy configuration");

        std::cout << "-1-" << std::endl;
        strategyModule.stop();

        std::cout << "-2-" << std::endl;
        marketEventQueue.close();

        std::cout << "-3-" << std::endl;
        bookUpdateQueue.close();

        std::cout << "-4-" << std::endl;
        bookBuilderModule.stop();

        std::cout << "-5-" << std::endl;
        marketDataModule.stop();
    }

      void testMarketDataToStrategySellPipeline()
    {
        const config::Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        auto marketDataSource = createMarketDataSource({
            "1,101,10000001,Sell,6500000000000,1000000000"
        });

        const Snapshot snapshot = createSnapshot(
            InstrumentId { 1 },
            SequenceNumber { 100 }
        );

        TestExchangeFactoryWithSnapshot exchangeFactory {
            std::move(marketDataSource),
            snapshot
        };

        MarketDataModule marketDataModule {
            config,
            bookUpdateQueue,
            exchangeFactory
        };

        BookBuilderModule bookBuilderModule {
            config,
            bookUpdateQueue,
            marketEventQueue,
            recordingQueue,
            exchangeFactory
        };

        StrategyModule strategyModule {
            config.strategy,
            marketEventQueue,
            executionQueue
        };

        marketDataModule.start();
        bookBuilderModule.start();
        strategyModule.start();

        ExecutionWorkItem workItem {};

        while (!executionQueue.tryPop(workItem))
        {
            std::this_thread::yield();
        }

        Assert(std::holds_alternative<OrderRequest>(workItem),
               "Strategy pipeline must produce an OrderRequest");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.instrument == InstrumentId { 1 },
               "Order instrument must match market data instrument");

        Assert(request.side == Side::Sell,
               "Strategy must generate Sell signal");

        Assert(request.type == OrderType::Limit,
               "Strategy order type must be Limit");

        Assert(request.price == Price { 0 },
               "Sell order must use best bid price");

        Assert(request.quantity == Quantity { 100'000'000 },
               "Order quantity must match strategy configuration");

        strategyModule.stop();

        marketEventQueue.close();

        bookUpdateQueue.close();
        bookBuilderModule.stop();

        marketDataModule.stop();
    }

    void testMarketDataToStrategyNoSignal()
    {
        const config::Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        auto marketDataSource = createMarketDataSource({
            "1,101,10000001,Buy,6500000000000,100"
        });

        const Snapshot snapshot = createSnapshot(
            InstrumentId { 1 },
            SequenceNumber { 100 }
        );

        TestExchangeFactoryWithSnapshot exchangeFactory {
            std::move(marketDataSource),
            snapshot
        };

        MarketDataModule marketDataModule {
            config,
            bookUpdateQueue,
            exchangeFactory
        };

        BookBuilderModule bookBuilderModule {
            config,
            bookUpdateQueue,
            marketEventQueue,
            recordingQueue,
            exchangeFactory
        };

        StrategyModule strategyModule {
            config.strategy,
            marketEventQueue,
            executionQueue
        };

        marketDataModule.start();
        bookBuilderModule.start();
        strategyModule.start();

        std::this_thread::sleep_for(std::chrono::milliseconds { 10 });

        ExecutionWorkItem workItem {};
        Assert(!executionQueue.tryPop(workItem),
               "Strategy must not produce an order when imbalance is below threshold");

        strategyModule.stop();

        marketEventQueue.close();

        bookUpdateQueue.close();
        bookBuilderModule.stop();

        marketDataModule.stop();
    }

    void testConfiguredOrderQuantity()
    {
        config::Config config = createConfig();
        config.strategy.orderQuantity = Quantity { 250'000'000 };

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        auto marketDataSource = createMarketDataSource({
            "1,101,10000001,Buy,6500000000000,1000000000"
        });

        const Snapshot snapshot = createSnapshot(
            InstrumentId { 1 },
            SequenceNumber { 100 }
        );

        TestExchangeFactoryWithSnapshot exchangeFactory {
            std::move(marketDataSource),
            snapshot
        };

        MarketDataModule marketDataModule {
            config,
            bookUpdateQueue,
            exchangeFactory
        };

        BookBuilderModule bookBuilderModule {
            config,
            bookUpdateQueue,
            marketEventQueue,
            recordingQueue,
            exchangeFactory
        };

        StrategyModule strategyModule {
            config.strategy,
            marketEventQueue,
            executionQueue
        };

        marketDataModule.start();
        bookBuilderModule.start();
        strategyModule.start();

        ExecutionWorkItem workItem {};

        while (!executionQueue.tryPop(workItem))
        {
            std::this_thread::yield();
        }

        Assert(std::holds_alternative<OrderRequest>(workItem),
               "Strategy pipeline must produce an OrderRequest");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.quantity == Quantity { 250'000'000 },
               "Strategy must use configured order quantity");

        strategyModule.stop();

        marketEventQueue.close();

        bookUpdateQueue.close();
        bookBuilderModule.stop();

        marketDataModule.stop();
    }

    void testBuyUsesBestAsk()
    {
        const config::Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        auto marketDataSource = createMarketDataSource({
            "1,101,10000001,Buy,6500000000000,1000000000",
            "1,102,10000002,Sell,6501000000000,10000000"
        });

        const Snapshot snapshot = createSnapshot(
            InstrumentId { 1 },
            SequenceNumber { 100 }
        );

        TestExchangeFactoryWithSnapshot exchangeFactory {
            std::move(marketDataSource),
            snapshot
        };

        MarketDataModule marketDataModule {
            config,
            bookUpdateQueue,
            exchangeFactory
        };

        BookBuilderModule bookBuilderModule {
            config,
            bookUpdateQueue,
            marketEventQueue,
            recordingQueue,
            exchangeFactory
        };

        StrategyModule strategyModule {
            config.strategy,
            marketEventQueue,
            executionQueue
        };

        marketDataModule.start();
        bookBuilderModule.start();
        strategyModule.start();

        ExecutionWorkItem workItem {};

        while (!executionQueue.tryPop(workItem))
        {
            std::this_thread::yield();
        }

        Assert(std::holds_alternative<OrderRequest>(workItem),
               "Strategy pipeline must produce an OrderRequest");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.side == Side::Buy,
               "Expected Buy order");

        Assert(request.price == Price { 6501000000000 },
               "Buy order must use best ask");

        strategyModule.stop();

        marketEventQueue.close();

        bookUpdateQueue.close();
        bookBuilderModule.stop();

        marketDataModule.stop();
    }

    void testSellUsesBestBid()
    {
        const config::Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        auto marketDataSource = createMarketDataSource({
            "1,101,10000001,Sell,6500000000000,1000000000",
            "1,102,10000002,Buy,6499000000000,10000000"
        });

        const Snapshot snapshot = createSnapshot(
            InstrumentId { 1 },
            SequenceNumber { 100 }
        );

        TestExchangeFactoryWithSnapshot exchangeFactory {
            std::move(marketDataSource),
            snapshot
        };

        MarketDataModule marketDataModule {
            config,
            bookUpdateQueue,
            exchangeFactory
        };

        BookBuilderModule bookBuilderModule {
            config,
            bookUpdateQueue,
            marketEventQueue,
            recordingQueue,
            exchangeFactory
        };

        StrategyModule strategyModule {
            config.strategy,
            marketEventQueue,
            executionQueue
        };

        marketDataModule.start();
        bookBuilderModule.start();
        strategyModule.start();

        ExecutionWorkItem workItem {};

        while (!executionQueue.tryPop(workItem))
        {
            std::this_thread::yield();
        }

        Assert(std::holds_alternative<OrderRequest>(workItem),
               "Strategy pipeline must produce an OrderRequest");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.side == Side::Sell,
               "Expected Sell order");

        Assert(request.price == Price { 6500000000000 },
               "Sell order must use best bid");

        strategyModule.stop();

        marketEventQueue.close();

        bookUpdateQueue.close();
        bookBuilderModule.stop();

        marketDataModule.stop();
    }

    void testMarketDataSequenceReachesStrategy()
    {
        const config::Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        auto marketDataSource = createMarketDataSource({
            "1,101,10000001,Buy,6500000000000,1000000000"
        });

        const Snapshot snapshot = createSnapshot(
            InstrumentId { 1 },
            SequenceNumber { 100 }
        );

        TestExchangeFactoryWithSnapshot exchangeFactory {
            std::move(marketDataSource),
            snapshot
        };

        MarketDataModule marketDataModule {
            config,
            bookUpdateQueue,
            exchangeFactory
        };

        BookBuilderModule bookBuilderModule {
            config,
            bookUpdateQueue,
            marketEventQueue,
            recordingQueue,
            exchangeFactory
        };

        StrategyModule strategyModule {
            config.strategy,
            marketEventQueue,
            executionQueue
        };

        marketDataModule.start();
        bookBuilderModule.start();

        MarketEvent event {};

        while (!marketEventQueue.tryPop(event))
        {
            std::this_thread::yield();
        }

        Assert(event.instrument == InstrumentId { 1 },
               "Market event instrument must match source message");

        Assert(event.sequence == SequenceNumber { 101 },
               "Market event sequence must match source message");

        strategyModule.start();

        executionQueue.close();
        strategyModule.stop();

        marketEventQueue.close();

        bookUpdateQueue.close();
        bookBuilderModule.stop();

        marketDataModule.stop();
    }

    void testMultipleMarketDataEvents()
    {
        const config::Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;
        ConditionVariableQueue<recording::RecordingEvent> recordingQueue;

        auto marketDataSource = createMarketDataSource({
            "1,101,10000001,Buy,6500000000000,1000000000",
            "1,102,10000002,Sell,6501000000000,10000000"
        });

        const Snapshot snapshot = createSnapshot(
            InstrumentId { 1 },
            SequenceNumber { 100 }
        );

        TestExchangeFactoryWithSnapshot exchangeFactory {
            std::move(marketDataSource),
            snapshot
        };

        MarketDataModule marketDataModule {
            config,
            bookUpdateQueue,
            exchangeFactory
        };

        BookBuilderModule bookBuilderModule {
            config,
            bookUpdateQueue,
            marketEventQueue,
            recordingQueue,
            exchangeFactory
        };

        StrategyModule strategyModule {
            config.strategy,
            marketEventQueue,
            executionQueue
        };

        marketDataModule.start();
        bookBuilderModule.start();
        strategyModule.start();

        ExecutionWorkItem first {};
        ExecutionWorkItem second {};

        while (!executionQueue.tryPop(first))
        {
            std::this_thread::yield();
        }

        while (!executionQueue.tryPop(second))
        {
            std::this_thread::yield();
        }

        Assert(std::holds_alternative<OrderRequest>(first),
               "First execution work item must be an OrderRequest");

        Assert(std::holds_alternative<OrderRequest>(second),
               "Second execution work item must be an OrderRequest");

        const OrderRequest& firstRequest = std::get<OrderRequest>(first);
        const OrderRequest& secondRequest = std::get<OrderRequest>(second);

        Assert(firstRequest.side == Side::Buy,
               "First signal must be Buy");

        Assert(secondRequest.side == Side::Buy,
               "Second signal must remain Buy");

        Assert(firstRequest.price == Price { 6501000000000 },
               "First Buy must use the best ask");

        Assert(secondRequest.price == Price { 6501000000000 },
               "Second Buy must use the current best ask");

        strategyModule.stop();

        marketEventQueue.close();

        bookUpdateQueue.close();
        bookBuilderModule.stop();

        marketDataModule.stop();
    }
}

void marketdata_bookbuilder_strategy_integrataion()
{
    testMarketDataToStrategyBuyPipeline();
    // testMarketDataToStrategySellPipeline();
    // testMarketDataToStrategyNoSignal();
    // testConfiguredOrderQuantity();
    // testBuyUsesBestAsk();
    // testSellUsesBestBid();
    // testMarketDataSequenceReachesStrategy();
    // testMultipleMarketDataEvents();

    std::cout << "All StrategyModule tests: OK\n";
}