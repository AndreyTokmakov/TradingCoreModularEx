/**============================================================================
Name        : book_builder_module_restore_book_test.cpp
Created on  : 05.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_builder_module.cpp
============================================================================**/

#include "book_builder_module.hpp"
#include "test_support/testing.hpp"

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
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
    using testing::AssertEqual;
    using testing::AssertTrue;

    template <typename Ty>
    using Queue = trading::concurrency::Queue<Ty>;

    constexpr InstrumentId INSTRUMENT { 42 };
    constexpr Price INITIAL_BID { 6'500'000'000'000 };
    constexpr Quantity INITIAL_BID_QUANTITY { 120'000'000 };
    constexpr Price INITIAL_ASK { 6'500'001'000'000 };
    constexpr Quantity INITIAL_ASK_QUANTITY { 90'000'000 };

    class TestSnapshotProvider final : public ISnapshotProvider
    {
    public:
        explicit TestSnapshotProvider(Snapshot snapshot,
                                      const std::chrono::milliseconds get_snapshot_delay =  std::chrono::milliseconds(0)) noexcept :
            snapshot { std::move(snapshot) },
            snapshotDelay {get_snapshot_delay} {
        }

        [[nodiscard]]
        Snapshot getSnapshot() override
        {
            ++snapshotRequestedCount;
            if (snapshotDelay.count() > 0) {
                std::this_thread::sleep_for(snapshotDelay);
            }
            return snapshot;
        }

        [[nodiscard]]
        uint32_t getSnapshotRequestedCount() const noexcept {
            return snapshotRequestedCount;
        }

    private:
        Snapshot snapshot;

        std::chrono::milliseconds snapshotDelay {};

        uint32_t snapshotRequestedCount { 0 };
    };

    class BlockingTestExchangeFactory final : public IExchangeFactory
    {
    public:
        explicit BlockingTestExchangeFactory(std::unique_ptr<TestSnapshotProvider> snapshotProvider) noexcept :
            snapshotProvider { std::move(snapshotProvider) }
        {
        }

        [[nodiscard]]
        std::unique_ptr<trading::execution::IExecutionGateway>
        createExecutionGateway(const Config&) const noexcept override {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<trading::execution::IExecutionReportSource>
        createExecutionReportSource(const Config&,
                                    Queue<trading::execution::ExecutionWorkItem>&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<IMarketDataParser>
        createMarketDataParser(const Config&) const noexcept override {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<IMarketDataSource>
        createMarketDataSource(const Config&) const noexcept override {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<ISnapshotProvider>
        createSnapshotProvider(const Config&) const noexcept override {
            return std::move(snapshotProvider);
        }

        [[nodiscard]]
        TestSnapshotProvider* getSnapshotProvider() const noexcept {
            return snapshotProvider.get();
        }

    private:
        mutable std::unique_ptr<TestSnapshotProvider> snapshotProvider;
    };


    Config createConfig()
    {
        Config config;
        config.instrument = INSTRUMENT;
        return config;
    }


    Snapshot createSnapshot(const SequenceNumber sequenceNumber = SequenceNumber { 100 })
    {
        return Snapshot {
            .instrument = INSTRUMENT,
            .sequence = sequenceNumber,
            .exchangeTimestamp = Timestamp { 1'000'000 },
            .bids = {
                { INITIAL_BID, INITIAL_BID_QUANTITY }
            },
            .asks = {
                { INITIAL_ASK, INITIAL_ASK_QUANTITY }
            }
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
}

namespace
{
    void testUpdatesArriveWhileSnapshotIsBeingFetched()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        std::unique_ptr<TestSnapshotProvider> snapshotProvider = std::make_unique<TestSnapshotProvider>(
            createSnapshot(), std::chrono::milliseconds(100)
        );
        TestSnapshotProvider* snapshotProviderPtr = snapshotProvider.get();
        BlockingTestExchangeFactory exchangeFactory { std::move(snapshotProvider) };
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory,};

        module.start();

        bookUpdateQueue.push(BookUpdates {
            createBidUpdate(SequenceNumber { 101 },Quantity { 200'000'000 })
        });

        bookUpdateQueue.push(BookUpdates {
            createAskUpdate(SequenceNumber { 102 },Quantity { 150'000'000 })
        });

        MarketEvent marketEvent;
        {   // GetSnapshotDelay - 100 milliseconds
            Assert(!strategyEventQueue.tryPop(marketEvent),"updates must not be processed before snapshot is applied");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        AssertEqual(1U, snapshotProviderPtr->getSnapshotRequestedCount(), "Snapshot shall be call once at this moment");

        {
            Assert(strategyEventQueue.waitPop(marketEvent),"update with snapshot sequence + 1 must be processed");
            Assert(marketEvent.sequence == SequenceNumber { 101 },"first update must have sequence 101");
            Assert(marketEvent.bestBidQuantity == Quantity { 200'000'000 },"first update must modify the bid");
            Assert(marketEvent.bestAskQuantity == INITIAL_ASK_QUANTITY,"first update must preserve snapshot ask");
        }
        {
            Assert(strategyEventQueue.waitPop(marketEvent), "second sequential update must be processed");
            Assert(marketEvent.sequence == SequenceNumber { 102 }, "second update must have sequence 102");
            Assert(marketEvent.bestBidQuantity == Quantity { 200'000'000 }, "second update must preserve the first update");
            Assert(marketEvent.bestAskQuantity == Quantity { 150'000'000 }, "second update must modify the ask");
        }

        bookUpdateQueue.close();
    }

    void testOnlyUpdatesAfterSnapshotSequenceAreProcessed()
    {
        Config config = createConfig();

        ConditionVariableQueue<BookUpdates> bookUpdateQueue;
        ConditionVariableQueue<MarketEvent> strategyEventQueue;
        ConditionVariableQueue<RecordingEvent> recordingQueue;

        auto snapshotProvider = std::make_unique<TestSnapshotProvider>(createSnapshot(150));
        TestSnapshotProvider* snapshotProviderPtr = snapshotProvider.get();
        BlockingTestExchangeFactory exchangeFactory { std::move(snapshotProvider) };
        BookBuilderModule module { config, bookUpdateQueue, strategyEventQueue, recordingQueue, exchangeFactory};

        module.start();

        for (SequenceNumber sequence{100}; sequence <= SequenceNumber { 200 }  ; ++sequence) {
            bookUpdateQueue.push(BookUpdates {
                createBidUpdate(sequence, Quantity {static_cast<int64_t>(sequence)} )
            });
        }

        MarketEvent marketEvent;
        for (SequenceNumber sequence { 151 }; sequence <= SequenceNumber { 200 } ; ++sequence) {
            Assert(strategyEventQueue.waitPop(marketEvent), "update after snapshot sequence must be processed");
            Assert(marketEvent.sequence == sequence, "updates must be processed in sequence order");
            Assert(marketEvent.bestBidQuantity == Quantity {static_cast<int64_t>(sequence)}, "processed update must modify the bid");
        }

        Assert(!strategyEventQueue.tryPop(marketEvent), "updates up to snapshot sequence must not be processed");
        bookUpdateQueue.close();
    }
}

void book_builder_module_restore_book_test()
{
    testUpdatesArriveWhileSnapshotIsBeingFetched();
    // testOnlyUpdatesAfterSnapshotSequenceAreProcessed();

    std::cout << "All BookBuilderModule RestoreBook tests: OK\n";
}