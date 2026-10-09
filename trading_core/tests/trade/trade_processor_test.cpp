/**============================================================================
Name        : trade_processor_test.cpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Unit tests for TradeProcessor.
============================================================================**/

#include "trade/trade_processor.hpp"
#include "market_data/model/trade.hpp"
#include "test_support/testing.hpp"

#include <iostream>

namespace
{
    using trading::InstrumentId;
    using trading::Price;
    using trading::Quantity;
    using trading::SequenceNumber;
    using trading::Timestamp;
    using trading::Side;
    using trading::market_data::Trade;
    using trading::trade::TradeProcessor;

    constexpr InstrumentId Instrument { 42 };
    constexpr InstrumentId OtherInstrument { 99 };

    constexpr SequenceNumber FirstSequence { 100 };
    constexpr SequenceNumber SecondSequence { 101 };

    constexpr Timestamp FirstTimestamp { 1'000'000 };
    constexpr Timestamp SecondTimestamp { 1'000'001 };

    constexpr Price FirstPrice { 65'000'000 };
    constexpr Price SecondPrice { 65'001'000 };

    constexpr Quantity FirstQuantity { 100'000 };
    constexpr Quantity SecondQuantity { 200'000 };

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

    Trade createTradeForOtherInstrument()
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

    void assertTradeEqual(const Trade& expected, const Trade& actual)
    {
        testing::AssertEqual(expected.instrument, actual.instrument);
        testing::AssertEqual(expected.sequence, actual.sequence);
        testing::AssertEqual(expected.exchangeTimestamp, actual.exchangeTimestamp);
        testing::AssertEqual(expected.price, actual.price);
        testing::AssertEqual(expected.quantity, actual.quantity);
        testing::AssertEqual(expected.side, actual.side);
    }

    void testInitialState()
    {
        const TradeProcessor processor { Instrument };

        testing::AssertEqual(Quantity {}, processor.getLastTrade().quantity);
        testing::AssertEqual(InstrumentId { 0 }, processor.getLastTrade().instrument);
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 0 }, processor.getTradeCount());
    }

    void testBuyTradeIsProcessed()
    {
        TradeProcessor processor { Instrument };
        const Trade trade = createBuyTrade();

        processor.onTrade(trade);

        assertTradeEqual(trade, processor.getLastTrade());
        testing::AssertEqual(FirstQuantity, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 1 }, processor.getTradeCount());
    }

    void testSellTradeIsProcessed()
    {
        TradeProcessor processor { Instrument };
        const Trade trade = createSellTrade();

        processor.onTrade(trade);

        assertTradeEqual(trade, processor.getLastTrade());
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(SecondQuantity, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 1 }, processor.getTradeCount());
    }

    void testMultipleBuyTradesAccumulateBuyVolume()
    {
        TradeProcessor processor { Instrument };

        Trade firstTrade = createBuyTrade();
        Trade secondTrade = createBuyTrade();
        secondTrade.sequence = SecondSequence;
        secondTrade.exchangeTimestamp = SecondTimestamp;
        secondTrade.price = SecondPrice;
        secondTrade.quantity = SecondQuantity;

        processor.onTrade(firstTrade);
        processor.onTrade(secondTrade);

        assertTradeEqual(secondTrade, processor.getLastTrade());
        testing::AssertEqual(FirstQuantity + SecondQuantity, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 2 }, processor.getTradeCount());
    }

    void testMultipleSellTradesAccumulateSellVolume()
    {
        TradeProcessor processor { Instrument };

        const Trade firstTrade = createSellTrade();
        Trade secondTrade = createSellTrade();
        secondTrade.sequence = FirstSequence;
        secondTrade.exchangeTimestamp = FirstTimestamp;
        secondTrade.price = FirstPrice;
        secondTrade.quantity = FirstQuantity;

        processor.onTrade(firstTrade);
        processor.onTrade(secondTrade);

        assertTradeEqual(secondTrade, processor.getLastTrade());
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(FirstQuantity + SecondQuantity, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 2 }, processor.getTradeCount());
    }

    void testBuyAndSellVolumesAreTrackedIndependently()
    {
        TradeProcessor processor { Instrument };

        const Trade buyTrade = createBuyTrade();
        const Trade sellTrade = createSellTrade();

        processor.onTrade(buyTrade);
        processor.onTrade(sellTrade);

        assertTradeEqual(sellTrade, processor.getLastTrade());
        testing::AssertEqual(FirstQuantity, processor.getBuyVolume());
        testing::AssertEqual(SecondQuantity, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 2 }, processor.getTradeCount());
    }

    void testLastTradeIsReplacedByMostRecentTrade()
    {
        TradeProcessor processor { Instrument };

        const Trade firstTrade = createBuyTrade();
        const Trade secondTrade = createSellTrade();

        processor.onTrade(firstTrade);
        processor.onTrade(secondTrade);

        assertTradeEqual(secondTrade, processor.getLastTrade());
    }

    void testTradeSequenceIsPreserved()
    {
        TradeProcessor processor { Instrument };
        const Trade trade = createSellTrade();

        processor.onTrade(trade);

        testing::AssertEqual(trade.sequence, processor.getLastTrade().sequence);
    }

    void testTradeTimestampIsPreserved()
    {
        TradeProcessor processor { Instrument };
        const Trade trade = createBuyTrade();

        processor.onTrade(trade);

        testing::AssertEqual(trade.exchangeTimestamp, processor.getLastTrade().exchangeTimestamp);
    }

    void testTradePriceIsPreserved()
    {
        TradeProcessor processor { Instrument };
        const Trade trade = createBuyTrade();

        processor.onTrade(trade);

        testing::AssertEqual(trade.price, processor.getLastTrade().price);
    }

    void testTradeQuantityIsPreserved()
    {
        TradeProcessor processor { Instrument };
        const Trade trade = createSellTrade();

        processor.onTrade(trade);

        testing::AssertEqual(trade.quantity, processor.getLastTrade().quantity);
    }

    void testWrongInstrumentIsIgnored()
    {
        TradeProcessor processor { Instrument };
        const Trade trade = createTradeForOtherInstrument();

        processor.onTrade(trade);

        testing::AssertEqual(InstrumentId { 0 }, processor.getLastTrade().instrument);
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 0 }, processor.getTradeCount());
    }

    void testWrongInstrumentDoesNotReplaceLastTrade()
    {
        TradeProcessor processor { Instrument };

        const Trade validTrade = createBuyTrade();
        const Trade wrongInstrumentTrade = createTradeForOtherInstrument();

        processor.onTrade(validTrade);
        processor.onTrade(wrongInstrumentTrade);

        assertTradeEqual(validTrade, processor.getLastTrade());
        testing::AssertEqual(FirstQuantity, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 1 }, processor.getTradeCount());
    }

    void testWrongInstrumentDoesNotAffectVolumes()
    {
        TradeProcessor processor { Instrument };

        const Trade validTrade = createBuyTrade();
        const Trade wrongInstrumentTrade = createTradeForOtherInstrument();

        processor.onTrade(validTrade);
        processor.onTrade(wrongInstrumentTrade);

        testing::AssertEqual(FirstQuantity, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
    }

    void testWrongInstrumentDoesNotAffectTradeCount()
    {
        TradeProcessor processor { Instrument };

        processor.onTrade(createTradeForOtherInstrument());

        testing::AssertEqual(uint64_t { 0 }, processor.getTradeCount());
    }

    void testZeroQuantityBuyTradeIsProcessed()
    {
        TradeProcessor processor { Instrument };

        Trade trade = createBuyTrade();
        trade.quantity = Quantity { 0 };

        processor.onTrade(trade);

        assertTradeEqual(trade, processor.getLastTrade());
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 1 }, processor.getTradeCount());
    }

    void testZeroQuantitySellTradeIsProcessed()
    {
        TradeProcessor processor { Instrument };

        Trade trade = createSellTrade();
        trade.quantity = Quantity { 0 };

        processor.onTrade(trade);

        assertTradeEqual(trade, processor.getLastTrade());
        testing::AssertEqual(Quantity {}, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(uint64_t { 1 }, processor.getTradeCount());
    }

    void testManyTrades()
    {
        TradeProcessor processor { Instrument };

        constexpr size_t TradeCount { 100 };

        for (size_t index = 0; index < TradeCount; ++index)
        {
            Trade trade = createBuyTrade();
            trade.sequence = SequenceNumber { FirstSequence + index };
            trade.quantity = FirstQuantity;

            processor.onTrade(trade);
        }

        testing::AssertEqual(FirstQuantity * TradeCount, processor.getBuyVolume());
        testing::AssertEqual(Quantity {}, processor.getSellVolume());
        testing::AssertEqual(uint64_t { TradeCount }, processor.getTradeCount());
        testing::AssertEqual(SequenceNumber { FirstSequence + TradeCount - 1 }, processor.getLastTrade().sequence);
    }
}

void trade_processor_test()
{
    testInitialState();
    testBuyTradeIsProcessed();
    testSellTradeIsProcessed();
    testMultipleBuyTradesAccumulateBuyVolume();
    testMultipleSellTradesAccumulateSellVolume();
    testBuyAndSellVolumesAreTrackedIndependently();
    testLastTradeIsReplacedByMostRecentTrade();
    testTradeSequenceIsPreserved();
    testTradeTimestampIsPreserved();
    testTradePriceIsPreserved();
    testTradeQuantityIsPreserved();
    testWrongInstrumentIsIgnored();
    testWrongInstrumentDoesNotReplaceLastTrade();
    testWrongInstrumentDoesNotAffectVolumes();
    testWrongInstrumentDoesNotAffectTradeCount();
    testZeroQuantityBuyTradeIsProcessed();
    testZeroQuantitySellTradeIsProcessed();
    testManyTrades();

    std::cout << "All TradeProcessor tests: OK\n";
}