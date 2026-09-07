/**============================================================================
Name        : market_data_module_test.cpp
Created on  : 06.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_data_module_test.cpp
============================================================================**/

#include "market_data_module.hpp"

#include "condition_variable_queue.hpp"
#include "test_support/test_exchange_factory.hpp"
#include "test_support/test_market_data_source.hpp"
#include "test_support/testing.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
    using testing::Assert;
    using trading::InstrumentId;
    using trading::Price;
    using trading::Quantity;
    using trading::Side;
    using trading::concurrency::ConditionVariableQueue;
    using trading::config::Config;
    using trading::market_data::BookUpdates;
    using trading::market_data::MarketDataModule;
    using trading::testing::TestExchangeFactory;
    using trading::testing::TestMarketDataSource;

    Config createConfig(const InstrumentId instrument)
    {
        Config config;
        config.instrument = instrument;
        return config;
    }

    void testMarketDataPipeline()
    {
        Config config = createConfig(InstrumentId { 1 });
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;

        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();
        marketDataSource->addTestMarketData({"1,101,10000001,Buy,6500000000000,120000000"});

        TestExchangeFactory exchangeFactory { std::move(marketDataSource) };
        MarketDataModule module { config, bookUpdateQueue, exchangeFactory};

        module.start();

        BookUpdates updates;
        const bool popped = bookUpdateQueue.tryPop(updates);

        Assert(popped, "parsed book updates must be pushed to queue");
        Assert(updates.size() == 1, "queue must contain one book update");

        const auto& update = updates.front();

        Assert(update.instrument == InstrumentId { 1 },"invalid instrument");
        Assert(update.sequence == 101,"invalid sequence");
        Assert(update.side == Side::Buy,"invalid side");
        Assert(update.price == Price { 6'500'000'000'000 },"invalid price");
        Assert(update.quantity == Quantity { 120'000'000 },"invalid quantity");

        module.stop();
    }

    void testMultipleMessages()
    {
        Config config = createConfig(InstrumentId { 1 });
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;

        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,10000001,Buy,6500000000000,120000000",
            "1,102,10000002,Sell,6500001000000,90000000",
            "1,103,10000003,Buy,6499999000000,250000000"
        });

        TestExchangeFactory exchangeFactory { std::move(marketDataSource) };
        MarketDataModule module { config, bookUpdateQueue, exchangeFactory };

        module.start();

        BookUpdates firstUpdates;
        BookUpdates secondUpdates;
        BookUpdates thirdUpdates;

        Assert(bookUpdateQueue.tryPop(firstUpdates),"first message updates must be pushed to queue");
        Assert(bookUpdateQueue.tryPop(secondUpdates),"second message updates must be pushed to queue");
        Assert(bookUpdateQueue.tryPop(thirdUpdates),"third message updates must be pushed to queue");
        Assert(firstUpdates.size() == 1, "first batch must contain one update");
        Assert(secondUpdates.size() == 1, "second batch must contain one update");
        Assert(thirdUpdates.size() == 1, "third batch must contain one update");
        Assert(firstUpdates.front().sequence == 101, "invalid first update sequence");
        Assert(firstUpdates.front().side == Side::Buy, "invalid first update side");
        Assert(secondUpdates.front().sequence == 102, "invalid second update sequence");
        Assert(secondUpdates.front().side == Side::Sell, "invalid second update side");
        Assert(thirdUpdates.front().sequence == 103, "invalid third update sequence");
        Assert(thirdUpdates.front().side == Side::Buy, "invalid third update side");
        Assert(bookUpdateQueue.empty(), "queue must contain exactly three update batches");

        module.stop();
    }

    void testMultipleMessages2()
    {
        Config config = createConfig(InstrumentId { 1 });
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;

        std::unique_ptr<TestMarketDataSource> marketDataSource =
            std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,10000001,Buy,6500000000000,120000000",
            "1,102,10000002,Sell,6500001000000,90000000",
            "1,103,10000003,Buy,6499999000000,250000000"
        });

        TestExchangeFactory exchangeFactory { std::move(marketDataSource) };
        MarketDataModule module { config, bookUpdateQueue, exchangeFactory };

        module.start();

        for (trading::SequenceNumber expectedSequence { 101 }; expectedSequence <= 103; ++expectedSequence)
        {
            BookUpdates updates;

            const bool popped = bookUpdateQueue.tryPop(updates);

            Assert(popped, "each market-data message must produce a queue item");
            Assert(updates.size() == 1, "each message must contain one book update");
            Assert(updates.front().sequence == expectedSequence, "invalid update sequence");
        }

        BookUpdates updates;
        Assert(!bookUpdateQueue.tryPop(updates),"queue must contain no unexpected updates");

        module.stop();
    }

    [[maybe_unused]]
    void testMultipleUpdatesInSingleMessage()
    {
        Config config = createConfig(InstrumentId { 1 });
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,10000001,Buy,6500000000000,120000000",
            "1,102,10000002,Sell,6500001000000,90000000"
        });

        TestExchangeFactory exchangeFactory { std::move(marketDataSource) };
        MarketDataModule module { config, bookUpdateQueue, exchangeFactory };

        module.start();

        BookUpdates updates;

        Assert(bookUpdateQueue.tryPop(updates),"parsed updates must be pushed to queue");
        Assert(updates.size() == 2, "batch must contain two updates");

        const auto& firstUpdate = updates[0];
        const auto& secondUpdate = updates[1];

        Assert(firstUpdate.instrument == InstrumentId { 1 }, "invalid first instrument");
        Assert(firstUpdate.sequence == 101, "invalid first sequence");
        Assert(firstUpdate.side == Side::Buy, "invalid first side");
        Assert(firstUpdate.price == Price { 6'500'000'000'000 }, "invalid first price");
        Assert(firstUpdate.quantity == Quantity { 120'000'000 }, "invalid first quantity");
        Assert(secondUpdate.instrument == InstrumentId { 1 }, "invalid second instrument");
        Assert(secondUpdate.sequence == 102, "invalid second sequence");
        Assert(secondUpdate.side == Side::Sell, "invalid second side");
        Assert(secondUpdate.price == Price { 6'500'001'000'000 }, "invalid second price");
        Assert(secondUpdate.quantity == Quantity { 90'000'000 }, "invalid second quantity");
        Assert(bookUpdateQueue.empty(), "all updates must be contained in one batch");

        module.stop();
    }

    void testInvalidMessageIsNotPushed()
    {
        Config config = createConfig(InstrumentId { 1 });
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "invalid message"
        });

        TestExchangeFactory exchangeFactory { std::move(marketDataSource) };
        MarketDataModule module { config, bookUpdateQueue, exchangeFactory };

        module.start();

        BookUpdates updates;

        Assert(!bookUpdateQueue.tryPop(updates),"invalid message must not produce queue updates");
        Assert(bookUpdateQueue.empty(), "queue must remain empty after invalid message");

        module.stop();
    }

    void testInvalidMessageDoesNotPreventNextMessage()
    {
        Config config = createConfig(InstrumentId { 1 });
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "invalid message",
            "1,101,10000001,Buy,6500000000000,120000000"
        });

        TestExchangeFactory exchangeFactory { std::move(marketDataSource) };
        MarketDataModule module { config, bookUpdateQueue, exchangeFactory };

        module.start();

        BookUpdates updates;

        Assert(bookUpdateQueue.tryPop(updates),"valid message after invalid message must still be processed");
        Assert(updates.size() == 1, "valid message must produce one update");

        const auto& update = updates.front();

        Assert(update.instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(update.sequence == 101, "invalid sequence");
        Assert(update.side == Side::Buy, "invalid side");
        Assert(update.price == Price { 6'500'000'000'000 }, "invalid price");
        Assert(update.quantity == Quantity { 120'000'000 }, "invalid quantity");
        Assert(bookUpdateQueue.empty(), "queue must contain only valid message updates");

        module.stop();
    }

    void testMixedValidAndInvalidMessages()
    {
        Config config = createConfig(InstrumentId { 1 });
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;

        std::unique_ptr<TestMarketDataSource> marketDataSource =
            std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,10000001,Buy,6500000000000,120000000",
            "invalid-message",
            "1,103,10000003,Sell,6500001000000,90000000"
        });

        TestExchangeFactory exchangeFactory { std::move(marketDataSource) };
        MarketDataModule module { config, bookUpdateQueue, exchangeFactory };

        module.start();

        BookUpdates updates;

        Assert(bookUpdateQueue.tryPop(updates),"first valid message must reach queue");
        Assert(updates.size() == 1, "first message must contain one update");
        Assert(updates.front().sequence == 101,"invalid sequence for first valid message");

        updates.clear();

        Assert(bookUpdateQueue.tryPop(updates),"second valid message must reach queue");
        Assert(updates.size() == 1, "second message must contain one update");
        Assert(updates.front().sequence == 103,"invalid sequence for second valid message");

        updates.clear();

        Assert(!bookUpdateQueue.tryPop(updates),"invalid message must not produce a queue item");
        module.stop();
    }

    [[maybe_unused]]
    void testStartProcessesMarketDataAgain()
    {
        Config config = createConfig(InstrumentId { 1 });
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,10000001,Buy,6500000000000,120000000"
        });

        TestExchangeFactory exchangeFactory { std::move(marketDataSource) };
        MarketDataModule module { config, bookUpdateQueue, exchangeFactory };

        module.start();

        BookUpdates firstUpdates;

        Assert(bookUpdateQueue.tryPop(firstUpdates),"first start must produce market data");
        Assert(firstUpdates.size() == 1, "first start must produce one update");
        Assert(firstUpdates.front().sequence == 101,"invalid sequence after first start");

        module.start();

        BookUpdates secondUpdates;

        Assert(bookUpdateQueue.tryPop(secondUpdates),"second start must process market data again");
        Assert(secondUpdates.size() == 1, "second start must produce one update");
        Assert(secondUpdates.front().sequence == 101,"invalid sequence after second start");

        BookUpdates updates;

        Assert(!bookUpdateQueue.tryPop(updates),"queue must contain no unexpected updates");

        module.stop();
    }
}

void market_data_module_test()
{
    testMarketDataPipeline();
    testMultipleMessages();
    testMultipleMessages2();
    // testMultipleUpdatesInSingleMessage(); /** NotSupported**/
    testInvalidMessageIsNotPushed();
    testInvalidMessageDoesNotPreventNextMessage();
    testMixedValidAndInvalidMessages();
    // testStartProcessesMarketDataAgain();

    std::cout << "All MarketDataModule tests: OK\n";
}