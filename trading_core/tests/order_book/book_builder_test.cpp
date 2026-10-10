/**============================================================================
Name        : book_builder_test.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BookBuilder unit tests.
============================================================================**/

#include "order_book/book_builder.hpp"
#include "common/common.hpp"
#include "market_data/market_event_dispatcher.hpp"
#include "recording/recording_event.hpp"
#include "test_support/testing.hpp"

#include <iostream>
#include <optional>
#include <variant>
#include <vector>

using trading::InstrumentId;
using trading::Price;
using trading::Quantity;
using trading::SequenceNumber;
using trading::Side;
using trading::Timestamp;

using trading::common::CVQueue;

using trading::market_data::BookLevel;
using trading::market_data::BookUpdate;
using trading::market_data::MarketEvent;
using trading::market_data::MarketEventDispatcher;
using trading::market_data::PriceLevelUpdate;
using trading::market_data::Snapshot;

using trading::order_book::BookBuilder;
using trading::order_book::OrderBook;

using trading::recording::RecordingEvent;

namespace
{
    using testing::Assert;
    using testing::AssertTrue;
    using testing::AssertFalse;
    using testing::AssertEqual;
    using testing::AssertNotEqual;
    using testing::AssertEmpty;

    constexpr InstrumentId Instrument { 42 };

    constexpr Price BidPrice { 6'500'000'000'000 };
    constexpr Price BidPrice2 { 6'499'999'000'000 };
    constexpr Price BidPrice3 { 6'499'998'000'000 };

    constexpr Price AskPrice { 6'500'001'000'000 };
    constexpr Price AskPrice2 { 6'500'002'000'000 };
    constexpr Price AskPrice3 { 6'500'003'000'000 };

    constexpr Quantity BidQuantity { 120'000'000 };
    constexpr Quantity BidQuantity2 { 250'000'000 };
    constexpr Quantity BidQuantity3 { 350'000'000 };

    constexpr Quantity AskQuantity { 90'000'000 };
    constexpr Quantity AskQuantity2 { 310'000'000 };
    constexpr Quantity AskQuantity3 { 410'000'000 };

    [[nodiscard]]
    PriceLevelUpdate createBidUpdate(const Price price, const Quantity quantity)
    {
        return PriceLevelUpdate {
            .side = Side::Buy,
            .price = price,
            .quantity = quantity
        };
    }

    [[nodiscard]]
    PriceLevelUpdate createAskUpdate(const Price price, const Quantity quantity)
    {
        return PriceLevelUpdate {
            .side = Side::Sell,
            .price = price,
            .quantity = quantity
        };
    }

    [[nodiscard]]
    BookUpdate createBookUpdate(const SequenceNumber first,
                                const SequenceNumber last,
                                std::vector<PriceLevelUpdate> updates)
    {
        return BookUpdate {
            .instrument = Instrument,
            .sequenceRange = {
                .first = first,
                .last = last
            },
            .exchangeTimestamp = Timestamp { 2'000'000 },
            .updates = std::move(updates)
        };
    }

    [[nodiscard]]
    MarketEvent popStrategyEvent(CVQueue<MarketEvent>& queue)
    {
        MarketEvent event;
        AssertTrue(queue.waitPop(event), "strategy queue must contain market event");
        return event;
    }

    [[nodiscard]]
    MarketEvent popRecordingEvent(CVQueue<RecordingEvent>& queue)
    {
        RecordingEvent recordingEvent;
        AssertTrue(queue.waitPop(recordingEvent), "recording queue must contain event");
        AssertTrue(std::holds_alternative<MarketEvent>(recordingEvent), "recording event must contain MarketEvent");
        return std::get<MarketEvent>(recordingEvent);
    }

    /*
    [[nodiscard]]
    BookBuilder createBuilder(OrderBook& orderBook,
                              CVQueue<MarketEvent>& strategyQueue,
                              CVQueue<RecordingEvent>& recordingQueue)
    {
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        return BookBuilder { Instrument, orderBook, dispatcher };
    }
    */

    void testApplySnapshot()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        const BookBuilder builder { Instrument, orderBook, dispatcher };

        constexpr Timestamp exchangeTimestamp { 1'000'000 };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = exchangeTimestamp,
            .bids = {
                { BidPrice, BidQuantity },
                { BidPrice2, BidQuantity2 }
            },
            .asks = {
                { AskPrice, AskQuantity },
                { AskPrice2, AskQuantity2 }
            }
        };

        const bool applied = builder.applySnapshot(snapshot);

        AssertTrue(applied, "snapshot must be applied");
        AssertEqual(orderBook.sequence(), SequenceNumber { 100 }, "invalid order book sequence");

        const std::optional<BookLevel> bestBid = orderBook.bestBid();
        const std::optional<BookLevel> bestAsk = orderBook.bestAsk();

        AssertTrue(bestBid.has_value(), "best bid must exist");
        AssertTrue(bestAsk.has_value(), "best ask must exist");

        AssertEqual(bestBid->price, BidPrice, "invalid best bid price");
        AssertEqual(bestBid->quantity, BidQuantity, "invalid best bid quantity");

        AssertEqual(bestAsk->price, AskPrice, "invalid best ask price");
        AssertEqual(bestAsk->quantity, AskQuantity, "invalid best ask quantity");

        Assert(strategyQueue.empty(), "strategy queue must be empty");
        Assert(recordingQueue.empty(), "recording queue must be empty");

        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testSnapshotDoesNotPublishMarketEvent()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        const BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        Assert(builder.applySnapshot(snapshot), "snapshot must be applied");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testBookUpdatePublishesMarketEvent()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 101 },
            SequenceNumber { 101 },
            { createBidUpdate(BidPrice, Quantity { 200'000'000 }) }));

        const MarketEvent strategyEvent = popStrategyEvent(strategyQueue);

        AssertEqual(strategyEvent.instrument, Instrument, "invalid event instrument");
        AssertEqual(strategyEvent.sequence, SequenceNumber { 101 }, "invalid event sequence");
        AssertEqual(strategyEvent.exchangeTimestamp, Timestamp { 2'000'000 },"invalid exchange timestamp");
        AssertEqual(strategyEvent.bestBid, BidPrice, "invalid best bid");
        AssertEqual(strategyEvent.bestBidQuantity, Quantity { 200'000'000 },"invalid best bid quantity");
        AssertEqual(strategyEvent.bestAsk, AskPrice, "invalid best ask");
        AssertEqual(strategyEvent.bestAskQuantity, AskQuantity, "invalid best ask quantity");

        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);

        AssertEqual(recordingEvent.instrument, strategyEvent.instrument,"recording event instrument mismatch");
        AssertEqual(recordingEvent.sequence, strategyEvent.sequence,"recording event sequence mismatch");
        AssertEqual(recordingEvent.bestBid, strategyEvent.bestBid,"recording event best bid mismatch");
        AssertEqual(recordingEvent.bestBidQuantity, strategyEvent.bestBidQuantity,"recording event best bid quantity mismatch");
        AssertEqual(recordingEvent.bestAsk, strategyEvent.bestAsk,"recording event best ask mismatch");
        AssertEqual(recordingEvent.bestAskQuantity, strategyEvent.bestAskQuantity,"recording event best ask quantity mismatch");

        AssertEqual(orderBook.sequence(), SequenceNumber { 101 },"order book sequence must advance to update last sequence");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testAskUpdate()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 101 },
            SequenceNumber { 101 },
            { createAskUpdate(AskPrice, Quantity { 150'000'000 }) }));

        const MarketEvent event = popStrategyEvent(strategyQueue);

        AssertEqual(event.sequence, SequenceNumber { 101 }, "invalid event sequence");
        AssertEqual(event.bestBid, BidPrice, "best bid must remain unchanged");
        AssertEqual(event.bestBidQuantity, BidQuantity,"best bid quantity must remain unchanged");
        AssertEqual(event.bestAsk, AskPrice, "invalid best ask");
        AssertEqual(event.bestAskQuantity, Quantity { 150'000'000 },"invalid best ask quantity");

        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);
        AssertEqual(recordingEvent.sequence, event.sequence,"recording event sequence mismatch");
        AssertEqual(orderBook.sequence(), SequenceNumber { 101 },"order book sequence must advance");
    }

    void testMultiLevelBookUpdate()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {},
            .asks = {}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 101 },
            SequenceNumber { 103 },
            {
                createBidUpdate(BidPrice, BidQuantity),
                createBidUpdate(BidPrice2, BidQuantity2),
                createAskUpdate(AskPrice, AskQuantity)
            }));

        AssertEqual(orderBook.sequence(), SequenceNumber { 103 },"order book sequence must equal update last sequence");
        AssertEqual(orderBook.bidVolume(BidPrice), BidQuantity,"first bid update must be applied");
        AssertEqual(orderBook.bidVolume(BidPrice2), BidQuantity2,"second bid update must be applied");
        AssertEqual(orderBook.askVolume(AskPrice), AskQuantity,"ask update must be applied");

        const MarketEvent event = popStrategyEvent(strategyQueue);

        AssertEqual(event.sequence, SequenceNumber { 103 }, "invalid event sequence");
        AssertEqual(event.bestBid, BidPrice, "invalid best bid");
        AssertEqual(event.bestBidQuantity, BidQuantity, "invalid best bid quantity");
        AssertEqual(event.bestAsk, AskPrice, "invalid best ask");
        AssertEqual(event.bestAskQuantity, AskQuantity, "invalid best ask quantity");

        [[maybe_unused]]
        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);
    }

    void testSequenceRangeAdvancesToLastSequence()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {},
            .asks = {}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 101 },
            SequenceNumber { 105 },
            { createBidUpdate(BidPrice, BidQuantity) }));

        AssertEqual(orderBook.sequence(), SequenceNumber { 105 }, "sequence must advance to range last value");

        const MarketEvent event = popStrategyEvent(strategyQueue);
        AssertEqual(event.sequence, SequenceNumber { 105 }, "published event must contain range last sequence");

        [[maybe_unused]]
        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);
    }

    void testSequenceGapDoesNotModifyBook()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 102 },
            SequenceNumber { 102 },
            { createBidUpdate(BidPrice, Quantity { 200'000'000 }) }));

        AssertEqual(orderBook.sequence(), SequenceNumber { 100 }, "sequence must not change after gap");
        AssertEqual(orderBook.bidVolume(BidPrice), BidQuantity, "book must not change after gap");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testSequenceGapWithRangeDoesNotModifyBook()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 102 },
            SequenceNumber { 105 },
            {
                createBidUpdate(BidPrice, Quantity { 200'000'000 }),
                createBidUpdate(BidPrice2, BidQuantity2)
            }));

        AssertEqual(orderBook.sequence(), SequenceNumber { 100 },"sequence must not change after sequence gap");
        AssertEqual(orderBook.bidVolume(BidPrice), BidQuantity,"book must not change after sequence gap");
        Assert(orderBook.bidVolume(BidPrice2).isZero(),"new levels must not be applied after sequence gap");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testStaleUpdateDoesNotModifyBook()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 100 },
            SequenceNumber { 100 },
            { createBidUpdate(BidPrice, Quantity { 200'000'000 }) }));

        AssertEqual(orderBook.sequence(), SequenceNumber { 100 },"sequence must not change after stale update");
        AssertEqual(orderBook.bidVolume(BidPrice), BidQuantity,"book must not change after stale update");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testOverlappingSequenceRangeIsRejected()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 100 },
            SequenceNumber { 101 },
            { createBidUpdate(BidPrice, Quantity { 200'000'000 }) }));

        AssertEqual(orderBook.sequence(), SequenceNumber { 100 },"sequence must not change after overlapping update");
        AssertEqual(orderBook.bidVolume(BidPrice), BidQuantity,"book must not change after overlapping update");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testValidUpdateAfterGapRecoverySnapshot()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot firstSnapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {}
        };

        AssertTrue(builder.applySnapshot(firstSnapshot), "first snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 102 },
            SequenceNumber { 102 },
            { createBidUpdate(BidPrice, Quantity { 200'000'000 }) }));

        AssertEqual(orderBook.sequence(), SequenceNumber { 100 }, "sequence must remain unchanged after gap");

        const Snapshot recoverySnapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 200 },
            .exchangeTimestamp = Timestamp { 3'000 },
            .bids = {{ BidPrice, BidQuantity2 }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        AssertTrue(builder.applySnapshot(recoverySnapshot), "recovery snapshot must be applied");
        AssertEqual(orderBook.sequence(), SequenceNumber { 200 },"recovery snapshot must replace sequence");
        AssertEqual(orderBook.bidVolume(BidPrice), BidQuantity2,"recovery snapshot must replace book state");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 201 },
            SequenceNumber { 201 },
            { createBidUpdate(BidPrice, BidQuantity3) }));

        AssertEqual(orderBook.sequence(), SequenceNumber { 201 },"update after recovery snapshot must be accepted");
        AssertEqual(orderBook.bidVolume(BidPrice), BidQuantity3,"update after recovery snapshot must modify book");

        const MarketEvent event = popStrategyEvent(strategyQueue);
        AssertEqual(event.sequence, SequenceNumber { 201 },"invalid event sequence after recovery");

        [[maybe_unused]]
        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);
    }

    void testUpdateAfterSnapshot()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        AssertTrue(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(createBookUpdate(
            SequenceNumber { 101 },
            SequenceNumber { 101 },
            { createBidUpdate(BidPrice, Quantity { 200'000'000 }) }));

        const MarketEvent event = popStrategyEvent(strategyQueue);

        AssertEqual(event.sequence, SequenceNumber { 101 }, "invalid final sequence");
        AssertEqual(event.exchangeTimestamp, Timestamp { 2'000'000 },"invalid final exchange timestamp");
        AssertEqual(event.bestBidQuantity, Quantity { 200'000'000 },"invalid final best bid quantity");

        [[maybe_unused]]
        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);
    }

    void testEmptySnapshot()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        const BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {},
            .asks = {}
        };

        const bool applied = builder.applySnapshot(snapshot);

        AssertTrue(applied, "empty snapshot must be applied");
        AssertEqual(orderBook.sequence(), SequenceNumber { 100 },"invalid order book sequence");
        AssertFalse(orderBook.bestBid().has_value(),"empty book must not have best bid");
        AssertFalse(orderBook.bestAsk().has_value(),"empty book must not have best ask");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testSnapshotWithWrongInstrumentIsRejected()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        const BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = InstrumentId { 2 },
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        AssertFalse(builder.applySnapshot(snapshot),"snapshot with wrong instrument must be rejected");
        AssertEqual(orderBook.sequence(), SequenceNumber { 0 },"order book sequence must remain unchanged");
        AssertTrue(orderBook.bidVolume(BidPrice).isZero(),"book must remain empty");
        AssertTrue(orderBook.askVolume(AskPrice).isZero(),"book must remain empty");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }

    void testBookUpdateWithWrongInstrumentIsIgnored()
    {
        OrderBook orderBook;
        CVQueue<MarketEvent> strategyQueue;
        CVQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher { strategyQueue, recordingQueue };
        const BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {},
            .asks = {}
        };

        Assert(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(BookUpdate {
            .instrument = InstrumentId { 2 },
            .sequenceRange = {
                .first = 101,
                .last = 101
            },
            .exchangeTimestamp = Timestamp { 2'000 },
            .updates = {
                createBidUpdate(BidPrice, Quantity { 100'000'000 })
            }
        });

        AssertEqual(orderBook.sequence(), SequenceNumber { 100 },"order book sequence must not change");
        AssertTrue(orderBook.bidVolume(BidPrice).isZero(),"wrong-instrument update must not change book");
        AssertEmpty(strategyQueue, "strategy queue must be empty");
        AssertEmpty(recordingQueue, "recording queue must be empty");
    }
}

void book_builder_test()
{
    testApplySnapshot();
    testSnapshotDoesNotPublishMarketEvent();
    testBookUpdatePublishesMarketEvent();
    testAskUpdate();

    testMultiLevelBookUpdate();
    testSequenceRangeAdvancesToLastSequence();

    testSequenceGapDoesNotModifyBook();
    testSequenceGapWithRangeDoesNotModifyBook();
    testStaleUpdateDoesNotModifyBook();
    testOverlappingSequenceRangeIsRejected();

    testValidUpdateAfterGapRecoverySnapshot();
    testUpdateAfterSnapshot();

    testEmptySnapshot();
    testSnapshotWithWrongInstrumentIsRejected();
    testBookUpdateWithWrongInstrumentIsIgnored();

    std::cout << "All BookBuilder tests: OK\n";
}