/**============================================================================
Name        : book_builder_test.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BookBuilder unit tests.
============================================================================**/

#include "book_builder.hpp"
#include "condition_variable_queue.hpp"
#include "market_event_dispatcher.hpp"
#include "recording_event.hpp"
#include "test_support/testing.hpp"

#include <iostream>
#include <variant>

using trading::InstrumentId;
using trading::Price;
using trading::Quantity;
using trading::SequenceNumber;
using trading::Side;
using trading::Timestamp;

using trading::concurrency::ConditionVariableQueue;

using trading::market_data::BookBuilder;
using trading::market_data::BookUpdate;
using trading::market_data::MarketEvent;
using trading::market_data::MarketEventDispatcher;
using trading::market_data::OrderBook;
using trading::market_data::Snapshot;

using trading::recording::RecordingEvent;

namespace
{
    using testing::Assert;
    using trading::market_data::BookLevel;

    constexpr InstrumentId Instrument { 42 };

    constexpr Price BidPrice { 6'500'000'000'000 };
    constexpr Price BidPrice2 { 6'499'999'000'000 };

    constexpr Price AskPrice { 6'500'001'000'000 };
    constexpr Price AskPrice2 { 6'500'002'000'000 };

    constexpr Quantity BidQuantity { 120'000'000 };
    constexpr Quantity BidQuantity2 { 250'000'000 };

    constexpr Quantity AskQuantity { 90'000'000 };
    constexpr Quantity AskQuantity2 { 310'000'000 };

    [[nodiscard]]
    MarketEvent popStrategyEvent(ConditionVariableQueue<MarketEvent>& queue)
    {
        MarketEvent event;
        Assert(queue.waitPop(event), "strategy queue must contain market event");
        return event;
    }

    [[nodiscard]]
    MarketEvent popRecordingEvent(ConditionVariableQueue<RecordingEvent>& queue)
    {
        RecordingEvent recordingEvent;
        Assert(queue.waitPop(recordingEvent), "recording queue must contain event");
        Assert(std::holds_alternative<MarketEvent>(recordingEvent),"recording event must contain MarketEvent");
        return std::get<MarketEvent>(recordingEvent);
    }

    void testApplySnapshot()
    {
        OrderBook orderBook;
        ConditionVariableQueue<MarketEvent> strategyQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher {strategyQueue,recordingQueue };
        const BookBuilder builder { Instrument,orderBook,dispatcher };

        constexpr Timestamp exchangeTimestamp { 1'000'000 };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = exchangeTimestamp,
            .bids = {{ BidPrice, BidQuantity },{ BidPrice2, BidQuantity2 }},
            .asks = {{ AskPrice, AskQuantity },{ AskPrice2, AskQuantity2 }}
        };

        const bool applied = builder.applySnapshot(snapshot);

        Assert(applied, "snapshot must be applied");
        Assert(orderBook.sequence() == SequenceNumber { 100 },"invalid order book sequence");

        const std::optional<BookLevel> bestBid = orderBook.bestBid();
        const std::optional<BookLevel> bestAsk = orderBook.bestAsk();

        Assert(bestBid.has_value(), "best bid must exist");
        Assert(bestAsk.has_value(), "best ask must exist");

        Assert(bestBid->price == BidPrice, "invalid best bid price");
        Assert(bestBid->quantity == BidQuantity, "invalid best bid quantity");

        Assert(bestAsk->price == AskPrice, "invalid best ask price");
        Assert(bestAsk->quantity == AskQuantity, "invalid best ask quantity");

        Assert(strategyQueue.empty(), "strategy queue must be empty");
        Assert(recordingQueue.empty(), "recording queue must be empty");
    }

    void testSnapshotDoesNotPublishMarketEvent()
    {
        OrderBook orderBook;
        ConditionVariableQueue<MarketEvent> strategyQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher {strategyQueue,recordingQueue };
        const BookBuilder builder { Instrument,orderBook,dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        const bool applied = builder.applySnapshot(snapshot);

        Assert(applied, "snapshot must be applied");
        Assert(strategyQueue.empty(), "strategy queue must be empty");
        Assert(recordingQueue.empty(), "recording queue must be empty");
    }

    void testBookUpdatePublishesMarketEvent()
    {
        OrderBook orderBook;
        ConditionVariableQueue<MarketEvent> strategyQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher {strategyQueue,recordingQueue};
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        Assert(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(BookUpdate {
            .instrument = Instrument,
            .sequence = SequenceNumber { 101 },
            .exchangeTimestamp = Timestamp { 2'000'000 },
            .side = Side::Buy,
            .price = BidPrice,
            .quantity = Quantity { 200'000'000 }
        });

        const MarketEvent strategyEvent = popStrategyEvent(strategyQueue);
        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);

        Assert(strategyEvent.instrument == Instrument, "invalid event instrument");
        Assert(strategyEvent.sequence == SequenceNumber { 101 },"invalid event sequence");
        Assert(strategyEvent.exchangeTimestamp == Timestamp { 2'000'000 },"invalid exchange timestamp");
        Assert(strategyEvent.bestBid == BidPrice, "invalid best bid");
        Assert(strategyEvent.bestBidQuantity == Quantity { 200'000'000 },"invalid best bid quantity");
        Assert(strategyEvent.bestAsk == AskPrice, "invalid best ask");
        Assert(strategyEvent.bestAskQuantity == AskQuantity,"invalid best ask quantity");
        Assert(recordingEvent.instrument == strategyEvent.instrument,"recording event instrument mismatch");
        Assert(recordingEvent.sequence == strategyEvent.sequence,"recording event sequence mismatch");
        Assert(recordingEvent.bestBid == strategyEvent.bestBid,"recording event best bid mismatch");
        Assert(recordingEvent.bestBidQuantity == strategyEvent.bestBidQuantity,"recording event best bid quantity mismatch");
        Assert(recordingEvent.bestAsk == strategyEvent.bestAsk,"recording event best ask mismatch");
        Assert(recordingEvent.bestAskQuantity == strategyEvent.bestAskQuantity,"recording event best ask quantity mismatch");

        Assert(strategyQueue.empty(), "strategy queue must be empty");
        Assert(recordingQueue.empty(), "recording queue must be empty");
    }

     void testAskUpdate()
    {
        OrderBook orderBook;
        ConditionVariableQueue<MarketEvent> strategyQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher {strategyQueue,recordingQueue};
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {{ AskPrice, AskQuantity }}
        };

        Assert(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(BookUpdate {
            .instrument = Instrument,
            .sequence = SequenceNumber { 101 },
            .exchangeTimestamp = Timestamp { 2'000'000 },
            .side = Side::Sell,
            .price = AskPrice,
            .quantity = Quantity { 150'000'000 }
        });

        const MarketEvent event = popStrategyEvent(strategyQueue);

        Assert(event.bestBid == BidPrice,"best bid must remain unchanged");
        Assert(event.bestBidQuantity == BidQuantity,"best bid quantity must remain unchanged");
        Assert(event.bestAsk == AskPrice,"invalid best ask");
        Assert(event.bestAskQuantity == Quantity { 150'000'000 },"invalid best ask quantity");

        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);
        Assert(recordingEvent.sequence == event.sequence,"recording event sequence mismatch");
    }

    void testSequenceGapDoesNotPublishEvent()
    {
        OrderBook orderBook;
        ConditionVariableQueue<MarketEvent> strategyQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher {strategyQueue,recordingQueue};
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {}
        };

        Assert(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(BookUpdate {
            .instrument = Instrument,
            .sequence = SequenceNumber { 102 },
            .exchangeTimestamp = Timestamp { 2'000'000 },
            .side = Side::Buy,
            .price = BidPrice,
            .quantity = Quantity { 200'000'000 }
        });

        Assert(orderBook.sequence() == SequenceNumber { 100 },"order book sequence must not change");
        Assert(orderBook.bidVolume(BidPrice) == BidQuantity,"invalid bid quantity after sequence gap");
        Assert(strategyQueue.empty(), "strategy queue must be empty");
        Assert(recordingQueue.empty(), "recording queue must be empty");
    }

    void testUpdateAfterSnapshot()
    {
        OrderBook orderBook;
        ConditionVariableQueue<MarketEvent> strategyQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher {strategyQueue,recordingQueue};
        BookBuilder builder { Instrument, orderBook, dispatcher };

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {{ BidPrice, BidQuantity }},
            .asks = {
                { AskPrice, AskQuantity }
            }
        };

        Assert(builder.applySnapshot(snapshot), "snapshot must be applied");

        builder.onBookUpdate(BookUpdate {
            .instrument = Instrument,
            .sequence = SequenceNumber { 101 },
            .exchangeTimestamp = Timestamp { 2'000'000 },
            .side = Side::Buy,
            .price = BidPrice,
            .quantity = Quantity { 200'000'000 }
        });

        const MarketEvent event = popStrategyEvent(strategyQueue);

        Assert(event.sequence == SequenceNumber { 101 },"invalid final sequence");
        Assert(event.exchangeTimestamp == Timestamp { 2'000'000 },"invalid final exchange timestamp");
        Assert(event.bestBidQuantity == Quantity { 200'000'000 },"invalid final best bid quantity");

        [[maybe_unused]]
        const MarketEvent recordingEvent = popRecordingEvent(recordingQueue);
    }

    void testEmptySnapshot()
    {
        OrderBook orderBook;
        ConditionVariableQueue<MarketEvent> strategyQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher {strategyQueue,recordingQueue};
        const BookBuilder builder { Instrument,orderBook, dispatcher};

        const Snapshot snapshot {
            .instrument = Instrument,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {},
            .asks = {}
        };

        const bool applied = builder.applySnapshot(snapshot);

        Assert(applied, "empty snapshot must be applied");
        Assert(orderBook.sequence() == SequenceNumber { 100 },"invalid order book sequence");
        Assert(!orderBook.bestBid().has_value(),"empty book must not have best bid");
        Assert(!orderBook.bestAsk().has_value(),"empty book must not have best ask");
        Assert(strategyQueue.empty(), "strategy queue must be empty");
        Assert(recordingQueue.empty(), "recording queue must be empty");
    }

    void testBookUpdateWithWrongInstrumentIsIgnored()
    {
        OrderBook orderBook;
        ConditionVariableQueue<MarketEvent> strategyQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;
        MarketEventDispatcher dispatcher {strategyQueue,recordingQueue};
        const BookBuilder builder { Instrument,orderBook, dispatcher};

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
            .sequence = SequenceNumber { 101 },
            .exchangeTimestamp = Timestamp { 2'000 },
            .side = Side::Buy,
            .price = BidPrice,
            .quantity = Quantity { 100'000'000 }
        });

        Assert(orderBook.sequence() == SequenceNumber { 100 },"order book sequence must not change");
        Assert(orderBook.bidVolume(BidPrice).isZero(),"wrong-instrument update must not change book");
        Assert(strategyQueue.empty(), "strategy queue must be empty");
        Assert(recordingQueue.empty(), "recording queue must be empty");
    }
}

void book_builder_test()
{
    testApplySnapshot();
    testSnapshotDoesNotPublishMarketEvent();
    testBookUpdatePublishesMarketEvent();
    testAskUpdate();
    testSequenceGapDoesNotPublishEvent();
    testUpdateAfterSnapshot();
    testEmptySnapshot();
    testBookUpdateWithWrongInstrumentIsIgnored();

    std::cout << "All BookBuilder tests: OK\n";
}