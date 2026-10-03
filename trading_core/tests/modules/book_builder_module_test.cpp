/**============================================================================
Name        : book_builder_module_test.cpp
Created on  : 05.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_builder_module_test.cpp
============================================================================**/

#include "order_book/book_builder_module.hpp"
#include "test_support/testing.hpp"
#include "test_support/test_snapshot_provider.hpp"
#include "test_support/test_exchange_factory.hpp"

#include <iostream>
#include <memory>
#include <utility>
#include <variant>

using trading::InstrumentId;
using trading::Price;
using trading::Quantity;
using trading::SequenceNumber;
using trading::Side;
using trading::Timestamp;

using trading::config::Config;
using trading::exchanges::IExchangeFactory;

using trading::concurrency::ConditionVariableQueue;

using trading::market_data::BookUpdate;
using trading::market_data::IMarketDataParser;
using trading::market_data::IMarketDataSource;
using trading::market_data::ISnapshotProvider;
using trading::market_data::MarketEvent;
using trading::market_data::PriceLevelUpdate;
using trading::market_data::Snapshot;
using trading::order_book::BookBuilderModule;
using trading::recording::RecordingEvent;

namespace
{
    using testing::Assert;
    using testing::AssertTrue;
    using testing::AssertEqual;
    using testing::AssertNotEqual;
    using testing::AssertEmpty;

    constexpr InstrumentId INSTRUMENT { 42 };
    constexpr InstrumentId OTHER_INSTRUMENT { 999 };

    constexpr Price INITIAL_BID { 6'500'000'000'000 };
    constexpr Quantity INITIAL_BID_QUANTITY { 120'000'000 };

    constexpr Price INITIAL_ASK { 6'500'001'000'000 };
    constexpr Quantity INITIAL_ASK_QUANTITY { 90'000'000 };

    constexpr Price SECOND_BID { 6'499'999'000'000 };
    constexpr Quantity SECOND_BID_QUANTITY { 80'000'000 };

    constexpr Price SECOND_ASK { 6'500'002'000'000 };
    constexpr Quantity SECOND_ASK_QUANTITY { 70'000'000 };

    Config createConfig()
    {
        Config config;
        config.instrument = INSTRUMENT;
        return config;
    }

    Snapshot createSnapshot()
    {
        return Snapshot {
            .instrument = INSTRUMENT,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {
                { INITIAL_BID, INITIAL_BID_QUANTITY }
            },
            .asks = {
                { INITIAL_ASK, INITIAL_ASK_QUANTITY }
            }
        };
    }

    [[maybe_unused]]
    Snapshot createSnapshotWithoutBid()
    {
        return Snapshot {
            .instrument = INSTRUMENT,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {},
            .asks = {
                { INITIAL_ASK, INITIAL_ASK_QUANTITY }
            }
        };
    }

    [[maybe_unused]]
    Snapshot createSnapshotWithoutAsk()
    {
        return Snapshot {
            .instrument = INSTRUMENT,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {
                { INITIAL_BID, INITIAL_BID_QUANTITY }
            },
            .asks = {}
        };
    }

    BookUpdate createBidUpdate(const SequenceNumber sequence,
                               const Quantity quantity)
    {
        return BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = sequence, .last = sequence },
            .exchangeTimestamp = Timestamp { sequence },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = INITIAL_BID,
                    .quantity = quantity
                }
            }
        };
    }

    BookUpdate createAskUpdate(const SequenceNumber sequence,
                               const Quantity quantity)
    {
        return BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = sequence, .last = sequence },
            .exchangeTimestamp = Timestamp { sequence },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Sell,
                    .price = INITIAL_ASK,
                    .quantity = quantity
                }
            }
        };
    }

    BookUpdate createSecondBidUpdate(const SequenceNumber sequence,
                                     const Quantity quantity)
    {
        return BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = sequence, .last = sequence },
            .exchangeTimestamp = Timestamp { sequence },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = SECOND_BID,
                    .quantity = quantity
                }
            }
        };
    }

    BookUpdate createSecondAskUpdate(const SequenceNumber sequence,
                                     const Quantity quantity)
    {
        return BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = sequence, .last = sequence },
            .exchangeTimestamp = Timestamp { sequence },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Sell,
                    .price = SECOND_ASK,
                    .quantity = quantity
                }
            }
        };
    }

    BookUpdate createWrongInstrumentUpdate(const SequenceNumber sequence,
                                           const Quantity quantity)
    {
        return BookUpdate {
            .instrument = OTHER_INSTRUMENT,
            .sequenceRange = { .first = sequence, .last = sequence },
            .exchangeTimestamp = Timestamp { sequence },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = INITIAL_BID,
                    .quantity = quantity
                }
            }
        };
    }

    void stopModule(ConditionVariableQueue<BookUpdate>& bookUpdateQueue)
    {
        bookUpdateQueue.close();
    }

    void testSnapshotIsRequested()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        Assert(exchangeFactory.createSnapshotProviderCountValue() == 1, "snapshot provider must be created once");

        bookUpdateQueue.close();
        module.run();

        Assert(exchangeFactory.createSnapshotProviderCountValue() == 1, "snapshot provider must remain created once");
    }

    void testSnapshotIsRequestedOnce()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot());
        auto snapshotProviderPtr = snapshotProvider.get();
        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::move(snapshotProvider)
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.close();
        module.run();

        AssertEqual(snapshotProviderPtr->getSnapshotRequestedCount(), 1UL, "snapshot must be requested exactly once");
    }

    void testSnapshotDoesNotProduceEvents()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.close();
        module.run();
    }

     void testSingleUpdateIsProcessed()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent), "one valid update must produce one strategy event");
        AssertEqual(marketEvent.instrument, INSTRUMENT, "invalid market event instrument");
        AssertEqual(marketEvent.sequence, SequenceNumber { 101 }, "invalid market event sequence");
        AssertEqual(marketEvent.exchangeTimestamp, Timestamp { 101 }, "invalid market event exchange timestamp");
        AssertEqual(marketEvent.bestBid, INITIAL_BID, "invalid best bid");
        AssertEqual(marketEvent.bestBidQuantity, Quantity { 200'000'000 }, "invalid best bid quantity");
        AssertEqual(marketEvent.bestAsk, INITIAL_ASK, "best ask must remain unchanged");
        AssertEqual(marketEvent.bestAskQuantity, INITIAL_ASK_QUANTITY, "best ask quantity must remain unchanged");

        RecordingEvent recordingEvent;

        AssertTrue(recordingQueue.waitPop(recordingEvent), "one valid update must produce one recording event");
        Assert(std::holds_alternative<MarketEvent>(recordingEvent), "recording event must contain MarketEvent");

        const auto& recordedMarketEvent = std::get<MarketEvent>(recordingEvent);

        AssertEqual(recordedMarketEvent.sequence, SequenceNumber { 101 }, "recorded event must preserve sequence");
        AssertEqual(recordedMarketEvent.bestBidQuantity, Quantity { 200'000'000 }, "recorded event must preserve bid quantity");
    }

    void testMultipleUpdatesInOneBatch()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = 101, .last = 102 },
            .exchangeTimestamp = Timestamp { 102 },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = INITIAL_BID,
                    .quantity = Quantity { 200'000'000 }
                },
                PriceLevelUpdate {
                    .side = Side::Sell,
                    .price = INITIAL_ASK,
                    .quantity = Quantity { 150'000'000 }
                }
            }
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent), "book update must produce market event");
        AssertEqual(marketEvent.sequence, SequenceNumber { 102 }, "event must use last sequence from sequence range");
        AssertEqual(marketEvent.bestBidQuantity, Quantity { 200'000'000 }, "event must contain updated bid quantity");
        AssertEqual(marketEvent.bestAskQuantity, Quantity { 150'000'000 }, "event must contain updated ask quantity");

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testMultipleBatchesAreProcessed()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000'000 }));
        bookUpdateQueue.push(createAskUpdate(SequenceNumber { 102 }, Quantity { 150'000'000 }));

        bookUpdateQueue.close();
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;

        AssertTrue(strategyEventQueue.waitPop(firstEvent), "first batch must produce event");
        AssertTrue(strategyEventQueue.waitPop(secondEvent), "second batch must produce event");
        AssertEqual(firstEvent.sequence, SequenceNumber { 101 }, "first batch event must have sequence 101");
        AssertEqual(secondEvent.sequence, SequenceNumber { 102 }, "second batch event must have sequence 102");
        AssertEqual(secondEvent.bestBidQuantity, Quantity { 200'000'000 }, "second batch must observe first batch update");
        AssertEqual(secondEvent.bestAskQuantity, Quantity { 150'000'000 }, "second batch must update ask");

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

        void testWrongInstrumentUpdateIsIgnored()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createWrongInstrumentUpdate(SequenceNumber { 101 }, Quantity { 500'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent), "valid update after wrong instrument update must be processed");
        AssertEqual(marketEvent.sequence, SequenceNumber { 101 }, "valid update must preserve its sequence");
        AssertEqual(marketEvent.bestBidQuantity, Quantity { 200'000 }, "wrong instrument update must not modify book");

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testSequenceGapDoesNotProduceEvent()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 102 }, Quantity { 200'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testDuplicateSequenceDoesNotProduceEvent()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 300'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent), "first update must produce event");
        AssertEqual(marketEvent.sequence, SequenceNumber { 101 }, "first event must have sequence 101");
        AssertEqual(marketEvent.bestBidQuantity, Quantity { 200'000 }, "first update quantity must be preserved");

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testOutOfOrderUpdateDoesNotAdvanceSequence()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 103 }, Quantity { 300'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 102 }, Quantity { 250'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 103 }, Quantity { 350'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;
        MarketEvent thirdEvent;

        AssertTrue(strategyEventQueue.waitPop(firstEvent), "sequence 101 must produce event");
        AssertTrue(strategyEventQueue.waitPop(secondEvent), "sequence 102 must produce event");
        AssertTrue(strategyEventQueue.waitPop(thirdEvent), "sequence 103 must produce event");
        AssertEqual(firstEvent.sequence, SequenceNumber { 101 }, "first event must have sequence 101");
        AssertEqual(secondEvent.sequence, SequenceNumber { 102 }, "second event must have sequence 102");
        AssertEqual(thirdEvent.sequence, SequenceNumber { 103 }, "third event must have sequence 103");
        AssertEqual(thirdEvent.bestBidQuantity, Quantity { 350'000 }, "latest valid update must determine final quantity");
        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testBidRemovalProducesEmptyBestBid()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity {}));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent), "bid removal must produce market event");
        AssertEqual(marketEvent.sequence, SequenceNumber { 101 }, "bid removal event must preserve sequence");
        AssertEqual(marketEvent.bestBid, Price {}, "best bid must be empty after bid removal");
        AssertEqual(marketEvent.bestBidQuantity, Quantity {}, "best bid quantity must be zero after bid removal");
        AssertEqual(marketEvent.bestAsk, INITIAL_ASK, "best ask must remain unchanged");
        AssertEqual(marketEvent.bestAskQuantity, INITIAL_ASK_QUANTITY, "best ask quantity must remain unchanged");
        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testAskRemovalProducesEmptyBestAsk()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createAskUpdate(SequenceNumber { 101 }, Quantity {}));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent), "ask removal must produce market event");
        AssertEqual(marketEvent.sequence, SequenceNumber { 101 }, "ask removal event must preserve sequence");
        AssertEqual(marketEvent.bestBid, INITIAL_BID, "best bid must remain unchanged");
        AssertEqual(marketEvent.bestBidQuantity, INITIAL_BID_QUANTITY, "best bid quantity must remain unchanged");
        AssertEqual(marketEvent.bestAsk, Price {}, "best ask must be empty after ask removal");
        AssertEqual(marketEvent.bestAskQuantity, Quantity {}, "best ask quantity must be zero after ask removal");
        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testBothSidesCanBeUpdated()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createSecondBidUpdate(SequenceNumber { 101 }, SECOND_BID_QUANTITY));
        bookUpdateQueue.push(createSecondAskUpdate(SequenceNumber { 102 }, SECOND_ASK_QUANTITY));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;

        AssertTrue(strategyEventQueue.waitPop(firstEvent), "bid update must produce event");
        AssertTrue(strategyEventQueue.waitPop(secondEvent), "ask update must produce event");
        AssertEqual(firstEvent.bestBid, INITIAL_BID, "initial bid must remain the best bid");
        AssertEqual(firstEvent.bestBidQuantity, INITIAL_BID_QUANTITY, "initial best bid quantity must remain unchanged");
        AssertEqual(firstEvent.bestAsk, INITIAL_ASK, "initial ask must remain the best ask");
        AssertEqual(firstEvent.bestAskQuantity, INITIAL_ASK_QUANTITY, "initial best ask quantity must remain unchanged");
        AssertEqual(secondEvent.bestBid, INITIAL_BID, "best bid must remain unchanged");
        AssertEqual(secondEvent.bestAsk, INITIAL_ASK, "initial ask must remain the best ask");
        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testRecordingReceivesSameNumberOfEvents()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000 }));
        bookUpdateQueue.push(createAskUpdate(SequenceNumber { 102 }, Quantity { 150'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 103 }, Quantity { 300'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;
        RecordingEvent recordingEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent), "first strategy event must exist");
        AssertTrue(strategyEventQueue.waitPop(marketEvent), "second strategy event must exist");
        AssertTrue(strategyEventQueue.waitPop(marketEvent), "third strategy event must exist");
        AssertTrue(recordingQueue.waitPop(recordingEvent), "first recording event must exist");
        AssertTrue(recordingQueue.waitPop(recordingEvent), "second recording event must exist");
        AssertTrue(recordingQueue.waitPop(recordingEvent), "third recording event must exist");

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
        AssertEmpty(recordingQueue, "recording event queue must be empty");
    }

    void testInvalidSnapshotStopsProcessing()
    {
        Snapshot invalidSnapshot {
            .instrument = OTHER_INSTRUMENT,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {{ INITIAL_BID, INITIAL_BID_QUANTITY }},
            .asks = {{ INITIAL_ASK, INITIAL_ASK_QUANTITY }}
        };

        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(invalidSnapshot)
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
        AssertEmpty(recordingQueue, "recording event queue must be empty");
    }

    void testEmptyBookUpdatesBatchDoesNotProduceEvent()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(BookUpdate {});

        stopModule(bookUpdateQueue);
        module.run();

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
        AssertEmpty(recordingQueue, "recording event queue must be empty");
    }

    void testMixedValidAndInvalidUpdates()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createWrongInstrumentUpdate(SequenceNumber { 101 }, Quantity { 500'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 103 }, Quantity { 300'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 102 }, Quantity { 250'000 }));
        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 103 }, Quantity { 350'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;
        MarketEvent thirdEvent;

        AssertTrue(strategyEventQueue.waitPop(firstEvent), "valid sequence 101 update must produce event");
        AssertTrue(strategyEventQueue.waitPop(secondEvent), "valid sequence 102 update must produce event");
        AssertTrue(strategyEventQueue.waitPop(thirdEvent), "valid sequence 103 update must produce event");
        AssertEqual(firstEvent.sequence, SequenceNumber { 101 }, "first valid event must have sequence 101");
        AssertEqual(secondEvent.sequence, SequenceNumber { 102 }, "second valid event must have sequence 102");
        AssertEqual(thirdEvent.sequence, SequenceNumber { 103 }, "third valid event must have sequence 103");
        AssertEqual(thirdEvent.bestBidQuantity, Quantity { 350'000 }, "final valid update must determine final bid quantity");

        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testReceiveTimestampIsGenerated()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent), "valid update must produce market event");
        AssertNotEqual(marketEvent.receiveTimestamp, Timestamp {}, "market event must contain receive timestamp");
        AssertEmpty(strategyEventQueue, "strategy event queue must be empty");
    }

    void testSingleBookUpdateWithMultipleLevelUpdatesProducesOneEvent()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = 101, .last = 103 },
            .exchangeTimestamp = Timestamp { 103 },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = INITIAL_BID,
                    .quantity = Quantity { 200'000'000 }
                },
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = SECOND_BID,
                    .quantity = SECOND_BID_QUANTITY
                },
                PriceLevelUpdate {
                    .side = Side::Sell,
                    .price = INITIAL_ASK,
                    .quantity = Quantity { 150'000'000 }
                }
            }
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent),"one BookUpdate must produce one strategy event");
        AssertEqual(marketEvent.sequence, SequenceNumber { 103 },"event sequence must equal BookUpdate last sequence");
        AssertEqual(marketEvent.bestBid, INITIAL_BID,"initial bid must remain best bid");
        AssertEqual(marketEvent.bestBidQuantity, Quantity { 200'000'000 },"best bid quantity must be updated");
        AssertEqual(marketEvent.bestAsk, INITIAL_ASK,"initial ask must remain best ask");
        AssertEqual(marketEvent.bestAskQuantity, Quantity { 150'000'000 },"best ask quantity must be updated");
        AssertEmpty(strategyEventQueue,"one BookUpdate must produce exactly one strategy event");

        RecordingEvent recordingEvent;

        AssertTrue(recordingQueue.waitPop(recordingEvent),"one BookUpdate must produce one recording event");
        Assert(std::holds_alternative<MarketEvent>(recordingEvent),"recording event must contain MarketEvent");
        AssertEmpty(recordingQueue,"one BookUpdate must produce exactly one recording event");
    }

    /**
        snapshot 100
        [101,105] -> sequence becomes 105
        [106,106] -> accepted
    **/
    void testSequenceRangeAdvancesToLastSequence()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = 101, .last = 105 },
            .exchangeTimestamp = Timestamp { 105 },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = INITIAL_BID,
                    .quantity = Quantity { 200'000'000 }
                },
                PriceLevelUpdate {
                    .side = Side::Sell,
                    .price = INITIAL_ASK,
                    .quantity = Quantity { 150'000'000 }
                }
            }
        });

        bookUpdateQueue.push(createBidUpdate(SequenceNumber { 106 }, Quantity { 250'000'000 }));

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;

        AssertTrue(strategyEventQueue.waitPop(firstEvent),"range update must produce event");
        AssertTrue(strategyEventQueue.waitPop(secondEvent),"next sequential update must produce event");
        AssertEqual(firstEvent.sequence, SequenceNumber { 105 },"first event must use sequenceRange.last");
        AssertEqual(secondEvent.sequence,SequenceNumber { 106 },"next update must start after sequenceRange.last");
        AssertEqual(secondEvent.bestBidQuantity, Quantity { 250'000'000 },"next update must observe previous BookUpdate");
        AssertEmpty(strategyEventQueue,"strategy event queue must be empty");
    }

    void testOverlappingSequenceRangeDoesNotProduceEvent()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdate> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { TestMocks {
            .snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot())
        }};
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        bookUpdateQueue.push(BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = 101, .last = 102 },
            .exchangeTimestamp = Timestamp { 102 },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = INITIAL_BID,
                    .quantity = Quantity { 200'000'000 }
                }
            }
        });

        bookUpdateQueue.push(BookUpdate {
            .instrument = INSTRUMENT,
            .sequenceRange = { .first = 102, .last = 103 },
            .exchangeTimestamp = Timestamp { 103 },
            .updates = {
                PriceLevelUpdate {
                    .side = Side::Buy,
                    .price = INITIAL_BID,
                    .quantity = Quantity { 300'000'000 }
                }
            }
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        AssertTrue(strategyEventQueue.waitPop(marketEvent),"first range update must produce event");
        AssertEqual(marketEvent.sequence,SequenceNumber { 102 },"first event must use range last sequence");
        AssertEqual(marketEvent.bestBidQuantity, Quantity { 200'000'000 },"first range update must be applied");
        AssertEmpty(strategyEventQueue,"overlapping range must not produce second event");
    }
}


void book_builder_module_test()
{
    testSnapshotIsRequested();
    testSnapshotIsRequestedOnce();
    testSnapshotDoesNotProduceEvents();
    testSingleUpdateIsProcessed();
    testMultipleUpdatesInOneBatch();
    testMultipleBatchesAreProcessed();
    testWrongInstrumentUpdateIsIgnored();
    testSequenceGapDoesNotProduceEvent();
    testDuplicateSequenceDoesNotProduceEvent();
    testOutOfOrderUpdateDoesNotAdvanceSequence();
    testBidRemovalProducesEmptyBestBid();
    testAskRemovalProducesEmptyBestAsk();
    testBothSidesCanBeUpdated();
    testRecordingReceivesSameNumberOfEvents();
    testInvalidSnapshotStopsProcessing();
    testEmptyBookUpdatesBatchDoesNotProduceEvent();
    testMixedValidAndInvalidUpdates();
    testReceiveTimestampIsGenerated();
    testSingleBookUpdateWithMultipleLevelUpdatesProducesOneEvent();
    testSequenceRangeAdvancesToLastSequence();
    testOverlappingSequenceRangeDoesNotProduceEvent();

    std::cout << "All BookBuilderModule tests: OK\n";
}