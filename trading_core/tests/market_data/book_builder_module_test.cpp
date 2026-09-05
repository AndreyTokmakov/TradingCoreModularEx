/**============================================================================
Name        : book_builder_module_test.cpp
Created on  : 05.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_builder_module_test.cpp
============================================================================**/

#include "book_builder_module.hpp"
#include "test_support/testing.hpp"

#include <iostream>
#include <memory>
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

using trading::market_data::BookBuilderModule;
using trading::market_data::BookUpdate;
using trading::market_data::BookUpdates;
using trading::market_data::IMarketDataParser;
using trading::market_data::IMarketDataSource;
using trading::market_data::ISnapshotProvider;
using trading::market_data::MarketEvent;
using trading::market_data::Snapshot;

using trading::recording::RecordingEvent;

namespace
{
    using testing::Assert;

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


    class TestSnapshotProvider final : public ISnapshotProvider
    {
    public:
        explicit TestSnapshotProvider(const Snapshot& snapshot,
                                      std::size_t& getSnapshotCount) noexcept :
            snapshot { snapshot },
            getSnapshotCount { getSnapshotCount }
        {
        }

        [[nodiscard]]
        Snapshot getSnapshot() override
        {
            ++getSnapshotCount;
            return snapshot;
        }

    private:
        Snapshot snapshot;
        std::size_t& getSnapshotCount;
    };


    class TestExchangeFactory final : public IExchangeFactory
    {
    public:
        explicit TestExchangeFactory(const Snapshot& snapshot) noexcept :
            snapshot { snapshot }
        {
        }

        [[nodiscard]]
        std::unique_ptr<trading::execution::IExecutionGateway>
        createExecutionGateway(const Config&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<trading::execution::IExecutionReportSource>
        createExecutionReportSource(
            const Config&,
            trading::concurrency::Queue<trading::execution::ExecutionWorkItem>&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<IMarketDataParser>
        createMarketDataParser(const Config&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<IMarketDataSource>
        createMarketDataSource(const Config&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<ISnapshotProvider>
        createSnapshotProvider(const Config&) const noexcept override
        {
            ++createSnapshotProviderCount;
            return std::make_unique<TestSnapshotProvider>(
                snapshot,
                getSnapshotCount);
        }

        [[nodiscard]]
        std::size_t createSnapshotProviderCountValue() const noexcept
        {
            return createSnapshotProviderCount;
        }

        [[nodiscard]]
        std::size_t getSnapshotCountValue() const noexcept
        {
            return getSnapshotCount;
        }

    private:
        Snapshot snapshot;

        mutable std::size_t createSnapshotProviderCount { 0 };
        mutable std::size_t getSnapshotCount { 0 };
    };


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
            .sequence = sequence,
            .exchangeTimestamp = Timestamp { sequence },
            .side = Side::Buy,
            .price = INITIAL_BID,
            .quantity = quantity
        };
    }


    BookUpdate createAskUpdate(const SequenceNumber sequence,
                               const Quantity quantity)
    {
        return BookUpdate {
            .instrument = INSTRUMENT,
            .sequence = sequence,
            .exchangeTimestamp = Timestamp { sequence },
            .side = Side::Sell,
            .price = INITIAL_ASK,
            .quantity = quantity
        };
    }


    BookUpdate createSecondBidUpdate(const SequenceNumber sequence,
                                     const Quantity quantity)
    {
        return BookUpdate {
            .instrument = INSTRUMENT,
            .sequence = sequence,
            .exchangeTimestamp = Timestamp { sequence },
            .side = Side::Buy,
            .price = SECOND_BID,
            .quantity = quantity
        };
    }

    BookUpdate createSecondAskUpdate(const SequenceNumber sequence,
                                     const Quantity quantity)
    {
        return BookUpdate {
            .instrument = INSTRUMENT,
            .sequence = sequence,
            .exchangeTimestamp = Timestamp { sequence },
            .side = Side::Sell,
            .price = SECOND_ASK,
            .quantity = quantity
        };
    }


    BookUpdate createWrongInstrumentUpdate(const SequenceNumber sequence,
                                           const Quantity quantity)
    {
        return BookUpdate {
            .instrument = OTHER_INSTRUMENT,
            .sequence = sequence,
            .exchangeTimestamp = Timestamp { sequence },
            .side = Side::Buy,
            .price = INITIAL_BID,
            .quantity = quantity
        };
    }


    void stopModule(ConditionVariableQueue<BookUpdates>& bookUpdateQueue)
    {
        bookUpdateQueue.close();
    }

    void testSnapshotIsRequested()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory };

        Assert(exchangeFactory.createSnapshotProviderCountValue() == 1, "snapshot provider must be created once");

        bookUpdateQueue.close();
        module.run();

        Assert(exchangeFactory.createSnapshotProviderCountValue() == 1, "snapshot provider must remain created once");
    }

    void testSnapshotIsRequestedOnce()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue,exchangeFactory};

        bookUpdateQueue.close();
        module.run();

        Assert(exchangeFactory.getSnapshotCountValue() == 1, "snapshot must be requested exactly once");
    }

    void testSnapshotDoesNotProduceEvents()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue,exchangeFactory};

        bookUpdateQueue.close();
        module.run();
    }

    void testSingleUpdateIsProcessed()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue,exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        Assert(strategyEventQueue.waitPop(marketEvent),"one valid update must produce one strategy event");
        Assert(marketEvent.instrument == INSTRUMENT,"invalid market event instrument");
        Assert(marketEvent.sequence == SequenceNumber { 101 },"invalid market event sequence");
        Assert(marketEvent.exchangeTimestamp == Timestamp { 101 },"invalid market event exchange timestamp");
        Assert(marketEvent.bestBid == INITIAL_BID,"invalid best bid");
        Assert(marketEvent.bestBidQuantity == Quantity { 200'000'000 },"invalid best bid quantity");
        Assert(marketEvent.bestAsk == INITIAL_ASK,"best ask must remain unchanged");
        Assert(marketEvent.bestAskQuantity == INITIAL_ASK_QUANTITY,"best ask quantity must remain unchanged");

        RecordingEvent recordingEvent;

        Assert(recordingQueue.waitPop(recordingEvent),"one valid update must produce one recording event");
        Assert(std::holds_alternative<MarketEvent>(recordingEvent),"recording event must contain MarketEvent");

        const auto& recordedMarketEvent = std::get<MarketEvent>(recordingEvent);

        Assert(recordedMarketEvent.sequence == SequenceNumber { 101 },"recorded event must preserve sequence");
        Assert(recordedMarketEvent.bestBidQuantity == Quantity { 200'000'000 },"recorded event must preserve bid quantity");
    }

    void testMultipleUpdatesInOneBatch()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue,exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000'000 }),
            createAskUpdate(SequenceNumber { 102 },Quantity { 150'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;

        Assert(strategyEventQueue.waitPop(firstEvent),"first update must produce market event");
        Assert(strategyEventQueue.waitPop(secondEvent),"second update must produce market event");
        Assert(firstEvent.sequence == SequenceNumber { 101 },"first event must have sequence 101");
        Assert(firstEvent.bestBidQuantity == Quantity { 200'000'000 },"first event must contain updated bid quantity");
        Assert(firstEvent.bestAskQuantity == INITIAL_ASK_QUANTITY,"first event must contain original ask quantity");
        Assert(secondEvent.sequence == SequenceNumber { 102 },"second event must have sequence 102");
        Assert(secondEvent.bestBidQuantity == Quantity { 200'000'000 },"second event must preserve updated bid quantity");
        Assert(secondEvent.bestAskQuantity == Quantity { 150'000'000 },"second event must contain updated ask quantity");

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testMultipleBatchesAreProcessed()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue,exchangeFactory};

        bookUpdateQueue.push(BookUpdates {createBidUpdate(
            SequenceNumber { 101 }, Quantity { 200'000'000 })
        });
        bookUpdateQueue.push(BookUpdates {createAskUpdate(
            SequenceNumber { 102 }, Quantity { 150'000'000 })
        });

        bookUpdateQueue.close();
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;

        Assert(strategyEventQueue.waitPop(firstEvent),"first batch must produce event");
        Assert(strategyEventQueue.waitPop(secondEvent),"second batch must produce event");
        Assert(firstEvent.sequence == SequenceNumber { 101 },"first batch event must have sequence 101");
        Assert(secondEvent.sequence == SequenceNumber { 102 },"second batch event must have sequence 102");
        Assert(secondEvent.bestBidQuantity == Quantity { 200'000'000 },"second batch must observe first batch update");
        Assert(secondEvent.bestAskQuantity == Quantity { 150'000'000 },"second batch must update ask");

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testWrongInstrumentUpdateIsIgnored()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createWrongInstrumentUpdate(SequenceNumber { 101 }, Quantity { 500'000'000 }),
            createBidUpdate(SequenceNumber { 101 },Quantity { 200'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        Assert(strategyEventQueue.waitPop(marketEvent), "valid update after wrong instrument update must be processed");
        Assert(marketEvent.sequence == SequenceNumber { 101 }, "valid update must preserve its sequence");
        Assert(marketEvent.bestBidQuantity == Quantity { 200'000'000 }, "wrong instrument update must not modify book");

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testSequenceGapDoesNotProduceEvent()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 102 },Quantity { 200'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testDuplicateSequenceDoesNotProduceEvent()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 }, Quantity { 200'000'000 }),
            createBidUpdate(SequenceNumber { 101 },Quantity { 300'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        Assert(strategyEventQueue.waitPop(marketEvent),"first update must produce event");
        Assert(marketEvent.sequence == SequenceNumber { 101 }, "first event must have sequence 101");
        Assert(marketEvent.bestBidQuantity == Quantity { 200'000'000 }, "first update quantity must be preserved");

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testOutOfOrderUpdateDoesNotAdvanceSequence()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 },Quantity { 200'000'000 }),
            createBidUpdate(SequenceNumber { 103 },Quantity { 300'000'000 }),
            createBidUpdate(SequenceNumber { 102 },Quantity { 250'000'000 }),
            createBidUpdate(SequenceNumber { 103 },Quantity { 350'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;
        MarketEvent thirdEvent;

        Assert(strategyEventQueue.waitPop(firstEvent),"sequence 101 must produce event");
        Assert(strategyEventQueue.waitPop(secondEvent),"sequence 102 must produce event");
        Assert(strategyEventQueue.waitPop(thirdEvent),"sequence 103 must produce event");
        Assert(firstEvent.sequence == SequenceNumber { 101 },"first event must have sequence 101");
        Assert(secondEvent.sequence == SequenceNumber { 102 },"second event must have sequence 102");
        Assert(thirdEvent.sequence == SequenceNumber { 103 },"third event must have sequence 103");
        Assert(thirdEvent.bestBidQuantity == Quantity { 350'000'000 },"latest valid update must determine final quantity");
        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testBidRemovalProducesEmptyBestBid()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 },Quantity {})
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        Assert(strategyEventQueue.waitPop(marketEvent),"bid removal must produce market event");
        Assert(marketEvent.sequence == SequenceNumber { 101 },"bid removal event must preserve sequence");
        Assert(marketEvent.bestBid == Price {},"best bid must be empty after bid removal");
        Assert(marketEvent.bestBidQuantity == Quantity {},"best bid quantity must be zero after bid removal");
        Assert(marketEvent.bestAsk == INITIAL_ASK,"best ask must remain unchanged");
        Assert(marketEvent.bestAskQuantity == INITIAL_ASK_QUANTITY,"best ask quantity must remain unchanged");
        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testAskRemovalProducesEmptyBestAsk()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createAskUpdate(SequenceNumber { 101 },Quantity {})
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        Assert(strategyEventQueue.waitPop(marketEvent),"ask removal must produce market event");
        Assert(marketEvent.sequence == SequenceNumber { 101 },"ask removal event must preserve sequence");
        Assert(marketEvent.bestBid == INITIAL_BID,"best bid must remain unchanged");
        Assert(marketEvent.bestBidQuantity == INITIAL_BID_QUANTITY,"best bid quantity must remain unchanged");
        Assert(marketEvent.bestAsk == Price {},"best ask must be empty after ask removal");
        Assert(marketEvent.bestAskQuantity == Quantity {},"best ask quantity must be zero after ask removal");
        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testBothSidesCanBeUpdated()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createSecondBidUpdate(SequenceNumber { 101 },SECOND_BID_QUANTITY),
            createSecondAskUpdate(SequenceNumber { 102 },SECOND_ASK_QUANTITY)
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;

        Assert(strategyEventQueue.waitPop(firstEvent),"bid update must produce event");
        Assert(strategyEventQueue.waitPop(secondEvent),"ask update must produce event");
        Assert(firstEvent.bestBid == INITIAL_BID,"initial bid must remain the best bid");
        Assert(firstEvent.bestBidQuantity == INITIAL_BID_QUANTITY,"initial best bid quantity must remain unchanged");
        Assert(firstEvent.bestAsk == INITIAL_ASK,"initial ask must remain the best ask");
        Assert(firstEvent.bestAskQuantity == INITIAL_ASK_QUANTITY,"initial best ask quantity must remain unchanged");
        Assert(secondEvent.bestBid == INITIAL_BID,"best bid must remain unchanged");
        Assert(secondEvent.bestAsk == INITIAL_ASK,"initial ask must remain the best ask");
        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testRecordingReceivesSameNumberOfEvents()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 },Quantity { 200'000'000 }),
            createAskUpdate(SequenceNumber { 102 },Quantity { 150'000'000 }),
            createBidUpdate(SequenceNumber { 103 },Quantity { 300'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;
        RecordingEvent recordingEvent;

        Assert(strategyEventQueue.waitPop(marketEvent),"first strategy event must exist");
        Assert(strategyEventQueue.waitPop(marketEvent),"second strategy event must exist");
        Assert(strategyEventQueue.waitPop(marketEvent),"third strategy event must exist");
        Assert(recordingQueue.waitPop(recordingEvent),"first recording event must exist");
        Assert(recordingQueue.waitPop(recordingEvent),"second recording event must exist");
        Assert(recordingQueue.waitPop(recordingEvent),"third recording event must exist");

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
        Assert(recordingQueue.empty(),"recording event queue must be empty");
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

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { invalidSnapshot };
        BookBuilderModule module {config,bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 },Quantity { 200'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
        Assert(recordingQueue.empty(),"recording event queue must be empty");
    }

    void testEmptyBookUpdatesBatchDoesNotProduceEvent()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {});

        stopModule(bookUpdateQueue);
        module.run();

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
        Assert(recordingQueue.empty(),"recording event queue must be empty");
    }

    void testMixedValidAndInvalidUpdates()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createWrongInstrumentUpdate(SequenceNumber { 101 },Quantity { 500'000'000 }),
            createBidUpdate(SequenceNumber { 101 },Quantity { 200'000'000 }),
            createBidUpdate(SequenceNumber { 103 },Quantity { 300'000'000 }),
            createBidUpdate(SequenceNumber { 102 },Quantity { 250'000'000 }),
            createBidUpdate(SequenceNumber { 103 },Quantity { 350'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent firstEvent;
        MarketEvent secondEvent;
        MarketEvent thirdEvent;

        Assert(strategyEventQueue.waitPop(firstEvent),"valid sequence 101 update must produce event");
        Assert(strategyEventQueue.waitPop(secondEvent),"valid sequence 102 update must produce event");
        Assert(strategyEventQueue.waitPop(thirdEvent),"valid sequence 103 update must produce event");
        Assert(firstEvent.sequence == SequenceNumber { 101 },"first valid event must have sequence 101");
        Assert(secondEvent.sequence == SequenceNumber { 102 },"second valid event must have sequence 102");
        Assert(thirdEvent.sequence == SequenceNumber { 103 },"third valid event must have sequence 103");
        Assert(thirdEvent.bestBidQuantity == Quantity { 350'000'000 },"final valid update must determine final bid quantity");

        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
    }

    void testReceiveTimestampIsGenerated()
    {
        Config config = createConfig();
        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        TestExchangeFactory exchangeFactory { createSnapshot() };
        BookBuilderModule module { config, bookUpdateQueue,strategyEventQueue,recordingQueue, exchangeFactory};

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 },Quantity { 200'000'000 })
        });

        stopModule(bookUpdateQueue);
        module.run();

        MarketEvent marketEvent;

        Assert(strategyEventQueue.waitPop(marketEvent),"valid update must produce market event");
        Assert(marketEvent.receiveTimestamp != Timestamp {},"market event must contain receive timestamp");
        Assert(strategyEventQueue.empty(),"strategy event queue must be empty");
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

    std::cout << "All BookBuilderModule tests: OK\n";
}