/**============================================================================
Name        : market_data_module_test.cpp
Created on  : 06.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_data_module_test.cpp
============================================================================**/

#include "market_data/market_data_module.hpp"

#include "common/common.hpp"
#include "test_support/test_exchange_factory.hpp"
#include "test_support/test_market_data_source.hpp"
#include "test_support/testing.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "test_support/test_market_data_parser.hpp"

namespace
{
    using testing::Assert;
    using trading::InstrumentId;
    using trading::Price;
    using trading::Quantity;
    using trading::Timestamp;
    using trading::Side;
    using trading::common::CVQueue;
    using trading::config::Config;
    using trading::market_data::BookUpdate;
    using trading::market_data::Trade;
    using trading::market_data::MarketDataModule;
    using trading::testing::TestMocks;
    using trading::testing::TestMarketDataParser;
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
        const Config config = createConfig(InstrumentId { 1 });
        CVQueue<BookUpdate> bookUpdateQueue;
        CVQueue<Trade> tradeQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,101,10000001,Buy,6500000000000,120000000"
        });

        TestExchangeFactory exchangeFactory { TestMocks {
            .marketDataParser = std::make_unique<TestMarketDataParser>(),
            .marketDataSource = std::move(marketDataSource),
        }};

        MarketDataModule module { config, bookUpdateQueue, tradeQueue, exchangeFactory };

        module.start();

        BookUpdate update;

        Assert(bookUpdateQueue.tryPop(update), "parsed book update must be pushed to queue");
        Assert(update.instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(update.sequenceRange.first == 101, "invalid first sequence");
        Assert(update.sequenceRange.last == 101, "invalid last sequence");
        Assert(update.exchangeTimestamp == trading::Timestamp { 10'000'001 }, "invalid exchange timestamp");
        Assert(update.updates.size() == 1, "book update must contain one price level update");

        const auto& levelUpdate = update.updates.front();

        Assert(levelUpdate.side == Side::Buy, "invalid side");
        Assert(levelUpdate.price == Price { 6'500'000'000'000 }, "invalid price");
        Assert(levelUpdate.quantity == Quantity { 120'000'000 }, "invalid quantity");

        module.stop();
    }

    void testMultipleMessages()
    {
        const Config config = createConfig(InstrumentId { 1 });
        CVQueue<BookUpdate> bookUpdateQueue;
        CVQueue<Trade> tradeQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,101,10000001,Buy,6500000000000,120000000",
            "1,102,102,10000002,Sell,6500001000000,90000000",
            "1,103,103,10000003,Buy,6499999000000,250000000"
        });

        TestExchangeFactory exchangeFactory { TestMocks {
            .marketDataParser = std::make_unique<TestMarketDataParser>(),
            .marketDataSource = std::move(marketDataSource),
        }};

        MarketDataModule module { config, bookUpdateQueue, tradeQueue, exchangeFactory };

        module.start();

        BookUpdate firstUpdate;
        BookUpdate secondUpdate;
        BookUpdate thirdUpdate;

        Assert(bookUpdateQueue.tryPop(firstUpdate), "first message must be pushed to queue");
        Assert(bookUpdateQueue.tryPop(secondUpdate), "second message must be pushed to queue");
        Assert(bookUpdateQueue.tryPop(thirdUpdate), "third message must be pushed to queue");

        Assert(firstUpdate.instrument == InstrumentId { 1 }, "invalid first instrument");
        Assert(firstUpdate.sequenceRange.first == 101, "invalid first sequence");
        Assert(firstUpdate.sequenceRange.last == 101, "invalid first last sequence");
        Assert(firstUpdate.exchangeTimestamp == Timestamp { 10'000'001 }, "invalid first timestamp");
        Assert(firstUpdate.updates.size() == 1, "first update must contain one price level update");
        Assert(firstUpdate.updates.front().side == Side::Buy, "invalid first update side");

        Assert(secondUpdate.instrument == InstrumentId { 1 }, "invalid second instrument");
        Assert(secondUpdate.sequenceRange.first == 102, "invalid second sequence");
        Assert(secondUpdate.sequenceRange.last == 102, "invalid second last sequence");
        Assert(secondUpdate.exchangeTimestamp == Timestamp { 10'000'002 }, "invalid second timestamp");
        Assert(secondUpdate.updates.size() == 1, "second update must contain one price level update");
        Assert(secondUpdate.updates.front().side == Side::Sell, "invalid second update side");

        Assert(thirdUpdate.instrument == InstrumentId { 1 }, "invalid third instrument");
        Assert(thirdUpdate.sequenceRange.first == 103, "invalid third sequence");
        Assert(thirdUpdate.sequenceRange.last == 103, "invalid third last sequence");
        Assert(thirdUpdate.exchangeTimestamp == Timestamp { 10'000'003 }, "invalid third timestamp");
        Assert(thirdUpdate.updates.size() == 1, "third update must contain one price level update");
        Assert(thirdUpdate.updates.front().side == Side::Buy, "invalid third update side");

        Assert(bookUpdateQueue.empty(), "queue must contain exactly three book updates");

        module.stop();
    }

    void testMultipleMessages2()
    {
        const Config config = createConfig(InstrumentId { 1 });
        CVQueue<BookUpdate> bookUpdateQueue;
        CVQueue<Trade> tradeQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,101,10000001,Buy,6500000000000,120000000",
            "1,102,102,10000002,Sell,6500001000000,90000000",
            "1,103,103,10000003,Buy,6499999000000,250000000"
        });

        TestExchangeFactory exchangeFactory { TestMocks {
            .marketDataParser = std::make_unique<TestMarketDataParser>(),
            .marketDataSource = std::move(marketDataSource),
        }};

        MarketDataModule module { config, bookUpdateQueue, tradeQueue, exchangeFactory };

        module.start();

        for (trading::SequenceNumber expectedSequence { 101 }; expectedSequence <= 103; ++expectedSequence)
        {
            BookUpdate update;

            const bool popped = bookUpdateQueue.tryPop(update);

            Assert(popped, "each market-data message must produce a queue item");
            Assert(update.instrument == InstrumentId { 1 }, "invalid instrument");
            Assert(update.sequenceRange.first == expectedSequence, "invalid first sequence");
            Assert(update.sequenceRange.last == expectedSequence, "invalid last sequence");
            Assert(update.updates.size() == 1, "each message must contain one price level update");
        }

        BookUpdate update;

        Assert(!bookUpdateQueue.tryPop(update), "queue must contain no unexpected updates");

        module.stop();
    }

    void testMultipleUpdatesInSingleMessage()
    {
        const Config config = createConfig(InstrumentId { 1 });
        CVQueue<BookUpdate> bookUpdateQueue;
        CVQueue<Trade> tradeQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,103,10000003,Buy,6500000000000,120000000,"
            "Sell,6500001000000,90000000,"
            "Buy,6499999000000,250000000"
        });

        TestExchangeFactory exchangeFactory { TestMocks {
            .marketDataParser = std::make_unique<TestMarketDataParser>(),
            .marketDataSource = std::move(marketDataSource),
        }};

        MarketDataModule module { config, bookUpdateQueue, tradeQueue, exchangeFactory };

        module.start();

        BookUpdate update;

        Assert(bookUpdateQueue.tryPop(update), "parsed BookUpdate must be pushed to queue");
        Assert(update.instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(update.sequenceRange.first == 101, "invalid first sequence");
        Assert(update.sequenceRange.last == 103, "invalid last sequence");
        Assert(update.exchangeTimestamp == Timestamp { 10'000'003 }, "invalid exchange timestamp");
        Assert(update.updates.size() == 3, "BookUpdate must contain three price level updates");

        const auto& firstUpdate = update.updates[0];
        const auto& secondUpdate = update.updates[1];
        const auto& thirdUpdate = update.updates[2];

        Assert(firstUpdate.side == Side::Buy, "invalid first side");
        Assert(firstUpdate.price == Price { 6'500'000'000'000 }, "invalid first price");
        Assert(firstUpdate.quantity == Quantity { 120'000'000 }, "invalid first quantity");

        Assert(secondUpdate.side == Side::Sell, "invalid second side");
        Assert(secondUpdate.price == Price { 6'500'001'000'000 }, "invalid second price");
        Assert(secondUpdate.quantity == Quantity { 90'000'000 }, "invalid second quantity");

        Assert(thirdUpdate.side == Side::Buy, "invalid third side");
        Assert(thirdUpdate.price == Price { 6'499'999'000'000 }, "invalid third price");
        Assert(thirdUpdate.quantity == Quantity { 250'000'000 }, "invalid third quantity");

        Assert(bookUpdateQueue.empty(), "all updates must be contained in one BookUpdate");

        module.stop();
    }

    void testInvalidMessageIsNotPushed()
    {
        const Config config = createConfig(InstrumentId { 1 });
        CVQueue<BookUpdate> bookUpdateQueue;
        CVQueue<Trade> tradeQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({"invalid message"});

        TestExchangeFactory exchangeFactory { TestMocks {
            .marketDataParser = std::make_unique<TestMarketDataParser>(),
            .marketDataSource = std::move(marketDataSource),
        }};

        MarketDataModule module { config, bookUpdateQueue, tradeQueue, exchangeFactory };

        module.start();

        BookUpdate update;

        Assert(!bookUpdateQueue.tryPop(update),"invalid message must not produce a BookUpdate");
        Assert(bookUpdateQueue.empty(),"queue must remain empty after invalid message");

        module.stop();
    }

    void testInvalidMessageDoesNotPreventNextMessage()
    {
        const Config config = createConfig(InstrumentId { 1 });
        CVQueue<BookUpdate> bookUpdateQueue;
        CVQueue<Trade> tradeQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "invalid message",
            "1,101,101,10000001,Buy,6500000000000,120000000"
        });

        TestExchangeFactory exchangeFactory { TestMocks {
            .marketDataParser = std::make_unique<TestMarketDataParser>(),
            .marketDataSource = std::move(marketDataSource),
        }};

        MarketDataModule module { config, bookUpdateQueue, tradeQueue, exchangeFactory };
        module.start();

        BookUpdate update;

        Assert(bookUpdateQueue.tryPop(update), "valid message after invalid message must still be processed");
        Assert(update.instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(update.sequenceRange.first == 101, "invalid first sequence");
        Assert(update.sequenceRange.last == 101, "invalid last sequence");
        Assert(update.updates.size() == 1, "valid message must contain one price level update");

        const auto& levelUpdate = update.updates.front();

        Assert(levelUpdate.side == Side::Buy, "invalid side");
        Assert(levelUpdate.price == Price { 6'500'000'000'000 }, "invalid price");
        Assert(levelUpdate.quantity == Quantity { 120'000'000 }, "invalid quantity");

        Assert(bookUpdateQueue.empty(), "queue must contain only valid message updates");

        module.stop();
    }

    void testMixedValidAndInvalidMessages()
    {
        const Config config = createConfig(InstrumentId { 1 });
        CVQueue<BookUpdate> bookUpdateQueue;
        CVQueue<Trade> tradeQueue;
        std::unique_ptr<TestMarketDataSource> marketDataSource = std::make_unique<TestMarketDataSource>();

        marketDataSource->addTestMarketData({
            "1,101,101,10000001,Buy,6500000000000,120000000",
            "invalid-message",
            "1,103,103,10000003,Sell,6500001000000,90000000"
        });

        TestExchangeFactory exchangeFactory { TestMocks {
            .marketDataParser = std::make_unique<TestMarketDataParser>(),
            .marketDataSource = std::move(marketDataSource),
        }};

        MarketDataModule module { config, bookUpdateQueue, tradeQueue, exchangeFactory };

        module.start();

        BookUpdate update;

        Assert(bookUpdateQueue.tryPop(update), "first valid message must reach queue");
        Assert(update.instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(update.sequenceRange.first == 101, "invalid first sequence");
        Assert(update.sequenceRange.last == 101, "invalid last sequence");
        Assert(update.updates.size() == 1, "first message must contain one price level update");

        update.clear();

        Assert(bookUpdateQueue.tryPop(update), "second valid message must reach queue");
        Assert(update.instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(update.sequenceRange.first == 103, "invalid first sequence");
        Assert(update.sequenceRange.last == 103, "invalid last sequence");
        Assert(update.updates.size() == 1, "second message must contain one price level update");

        update.clear();

        Assert(!bookUpdateQueue.tryPop(update), "invalid message must not produce a queue item");

        module.stop();
    }
}

void market_data_module_test()
{
    testMarketDataPipeline();
    testMultipleMessages();
    testMultipleMessages2();
    testMultipleUpdatesInSingleMessage();
    testInvalidMessageIsNotPushed();
    testInvalidMessageDoesNotPreventNextMessage();
    testMixedValidAndInvalidMessages();

    std::cout << "All MarketDataModule tests: OK\n";
}