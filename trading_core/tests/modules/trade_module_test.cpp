/**============================================================================
Name        : trade_module_test.cpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Unit tests for TradeModule.
============================================================================**/

#include "trade/trade_module.hpp"
#include "common/common.hpp"
#include "config/config.hpp"
#include "market_data/model/trade.hpp"
#include "test_support/testing.hpp"

#include <iostream>

namespace
{
    using trading::InstrumentId;
    using trading::Price;
    using trading::Quantity;
    using trading::SequenceNumber;
    using trading::Side;
    using trading::Timestamp;
    using trading::config::Config;
    using trading::common::CVQueue;
    using trading::market_data::Trade;
    using trading::trade::TradeModule;

    constexpr InstrumentId Instrument { 42 };
    constexpr InstrumentId OtherInstrument { 99 };

    constexpr SequenceNumber FirstSequence { 100 };
    constexpr SequenceNumber SecondSequence { 101 };
    constexpr SequenceNumber ThirdSequence { 102 };

    constexpr Timestamp FirstTimestamp { 1'000'000 };
    constexpr Timestamp SecondTimestamp { 1'000'001 };
    constexpr Timestamp ThirdTimestamp { 1'000'002 };

    constexpr Price FirstPrice { 65'000'000 };
    constexpr Price SecondPrice { 65'001'000 };
    constexpr Price ThirdPrice { 65'002'000 };

    constexpr Quantity FirstQuantity { 100'000 };
    constexpr Quantity SecondQuantity { 200'000 };
    constexpr Quantity ThirdQuantity { 300'000 };

    Config createConfig()
    {
        Config config;
        config.instrument = Instrument;
        return config;
    }

    Trade createBuyTrade()
    {
        return Trade {
            .instrument = Instrument,
            .sequence = FirstSequence,
            .exchangeTimestamp = FirstTimestamp,
            .price = FirstPrice,
            .quantity = FirstQuantity,
            .side = Side::Buy
        };
    }

    Trade createSellTrade()
    {
        return Trade {
            .instrument = Instrument,
            .sequence = SecondSequence,
            .exchangeTimestamp = SecondTimestamp,
            .price = SecondPrice,
            .quantity = SecondQuantity,
            .side = Side::Sell
        };
    }

    Trade createSecondBuyTrade()
    {
        return Trade {
            .instrument = Instrument,
            .sequence = ThirdSequence,
            .exchangeTimestamp = ThirdTimestamp,
            .price = ThirdPrice,
            .quantity = ThirdQuantity,
            .side = Side::Buy
        };
    }

    Trade createWrongInstrumentTrade()
    {
        return Trade {
            .instrument = OtherInstrument,
            .sequence = FirstSequence,
            .exchangeTimestamp = FirstTimestamp,
            .price = FirstPrice,
            .quantity = FirstQuantity,
            .side = Side::Buy
        };
    }

    void testEmptyClosedQueueStopsModule()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.close();

        module.run();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 0 }, processor.getTradeCount());
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
    }

    void testSingleBuyTradeIsProcessed()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createBuyTrade());
        tradeQueue.close();

        module.run();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 1 }, processor.getTradeCount());
        testing::AssertEqual(FirstQuantity, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(FirstSequence, processor.getLastTrade().sequence);
    }

    void testSingleSellTradeIsProcessed()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createSellTrade());
        tradeQueue.close();

        module.run();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 1 }, processor.getTradeCount());
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(SecondQuantity, processor.getSellVolume());
        testing::AssertEqual(SecondSequence, processor.getLastTrade().sequence);
    }

    void testMultipleTradesAreProcessed()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createBuyTrade());
        tradeQueue.push(createSellTrade());
        tradeQueue.push(createSecondBuyTrade());
        tradeQueue.close();

        module.run();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 3 }, processor.getTradeCount());
        testing::AssertEqual(FirstQuantity + ThirdQuantity, processor.getBuyVolume());
        testing::AssertEqual(SecondQuantity, processor.getSellVolume());
        testing::AssertEqual(ThirdSequence, processor.getLastTrade().sequence);
    }

    void testTradesAreProcessedInQueueOrder()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        Trade firstTrade = createBuyTrade();
        Trade secondTrade = createSellTrade();
        Trade thirdTrade = createSecondBuyTrade();

        tradeQueue.push(firstTrade);
        tradeQueue.push(secondTrade);
        tradeQueue.push(thirdTrade);
        tradeQueue.close();

        module.run();

        testing::AssertEqual(thirdTrade.sequence, module.processor().getLastTrade().sequence);
        testing::AssertEqual(thirdTrade.exchangeTimestamp, module.processor().getLastTrade().exchangeTimestamp);
        testing::AssertEqual(thirdTrade.price, module.processor().getLastTrade().price);
        testing::AssertEqual(thirdTrade.quantity, module.processor().getLastTrade().quantity);
        testing::AssertEqual(thirdTrade.side, module.processor().getLastTrade().side);
        testing::AssertEqual(3UL, module.processor().getTradeCount());
    }

    void testWrongInstrumentTradeIsIgnored()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createWrongInstrumentTrade());
        tradeQueue.close();

        module.run();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 0 }, processor.getTradeCount());
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(InstrumentId { 0 }, processor.getLastTrade().instrument);
    }

    void testWrongInstrumentTradeDoesNotAffectValidTrades()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createBuyTrade());
        tradeQueue.push(createWrongInstrumentTrade());
        tradeQueue.push(createSellTrade());
        tradeQueue.close();

        module.run();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 2 }, processor.getTradeCount());
        testing::AssertEqual(FirstQuantity, processor.getBuyVolume());
        testing::AssertEqual(SecondQuantity, processor.getSellVolume());
        testing::AssertEqual(SecondSequence, processor.getLastTrade().sequence);
        testing::AssertEqual(2UL, module.processor().getTradeCount());
    }

    void testOnlyQueueTradesAreProcessed()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createBuyTrade());
        tradeQueue.close();

        module.run();

        testing::AssertTrue(tradeQueue.empty());
        testing::AssertEqual(uint64_t { 1 }, module.processor().getTradeCount());
    }

    void testQueueIsDrainedBeforeModuleStops()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createBuyTrade());
        tradeQueue.push(createSellTrade());
        tradeQueue.push(createSecondBuyTrade());
        tradeQueue.close();

        module.run();

        testing::AssertTrue(tradeQueue.empty());
        testing::AssertEqual(uint64_t { 3 }, module.processor().getTradeCount());
    }

    void testModuleCanRunUsingWorkerThread()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        module.start();

        tradeQueue.push(createBuyTrade());
        tradeQueue.push(createSellTrade());
        tradeQueue.close();

        module.stop();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 2 }, processor.getTradeCount());
        testing::AssertEqual(FirstQuantity, processor.getBuyVolume());
        testing::AssertEqual(SecondQuantity, processor.getSellVolume());
        testing::AssertEqual(SecondSequence, processor.getLastTrade().sequence);
    }

    void testModuleThreadProcessesAllTradesBeforeStop()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        module.start();

        constexpr size_t TradeCount { 100 };

        for (size_t index = 0; index < TradeCount; ++index)
        {
            Trade trade = createBuyTrade();
            trade.sequence = SequenceNumber { FirstSequence + index };
            trade.quantity = FirstQuantity;

            tradeQueue.push(trade);
        }

        tradeQueue.close();
        module.stop();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { TradeCount }, processor.getTradeCount());
        testing::AssertEqual(FirstQuantity * TradeCount, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(SequenceNumber { FirstSequence + TradeCount - 1 }, processor.getLastTrade().sequence);
    }

    void testModuleDoesNotProcessTradesForOtherInstrument()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createWrongInstrumentTrade());
        tradeQueue.push(createWrongInstrumentTrade());
        tradeQueue.close();

        module.run();

        testing::AssertEqual(uint64_t { 0 }, module.processor().getTradeCount());
        testing::AssertEqual(Quantity {}, module.processor().getBuyVolume());
        testing::AssertEqual(Quantity {}, module.processor().getSellVolume());
    }

    void testMixedInstrumentTradesAreHandledCorrectly()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        tradeQueue.push(createWrongInstrumentTrade());
        tradeQueue.push(createBuyTrade());
        tradeQueue.push(createWrongInstrumentTrade());
        tradeQueue.push(createSellTrade());
        tradeQueue.push(createWrongInstrumentTrade());
        tradeQueue.close();

        module.run();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 2 }, processor.getTradeCount());
        testing::AssertEqual(FirstQuantity, processor.getBuyVolume());
        testing::AssertEqual(SecondQuantity, processor.getSellVolume());
        testing::AssertEqual(SecondSequence, processor.getLastTrade().sequence);
    }

    void testModuleHandlesZeroQuantityTrade()
    {
        CVQueue<Trade> tradeQueue;
        TradeModule module { createConfig(), tradeQueue };

        Trade trade = createBuyTrade();
        trade.quantity = Quantity { 0 };

        tradeQueue.push(trade);
        tradeQueue.close();

        module.run();

        const auto& processor = module.processor();

        testing::AssertEqual(uint64_t { 1 }, processor.getTradeCount());
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(trade.sequence, processor.getLastTrade().sequence);
    }
}

void trade_module_test()
{
    testEmptyClosedQueueStopsModule();
    testSingleBuyTradeIsProcessed();
    testSingleSellTradeIsProcessed();
    testMultipleTradesAreProcessed();
    testTradesAreProcessedInQueueOrder();
    testWrongInstrumentTradeIsIgnored();
    testWrongInstrumentTradeDoesNotAffectValidTrades();
    testOnlyQueueTradesAreProcessed();
    testQueueIsDrainedBeforeModuleStops();
    testModuleCanRunUsingWorkerThread();
    testModuleThreadProcessesAllTradesBeforeStop();
    testModuleDoesNotProcessTradesForOtherInstrument();
    testMixedInstrumentTradesAreHandledCorrectly();
    testModuleHandlesZeroQuantityTrade();

    std::cout << "All TradeModule tests: OK" << std::endl;
}