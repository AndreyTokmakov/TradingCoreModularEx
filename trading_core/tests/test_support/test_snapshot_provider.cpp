/**============================================================================
Name        : test_snapshot_provider.cpp
Created on  : 25.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_snapshot_provider.cpp
============================================================================**/

#include "test_snapshot_provider.hpp"

#include <thread>
#include <utility>

namespace trading::testing
{

    TestSnapshotProvider::TestSnapshotProvider(market_data::Snapshot snapshot) noexcept :
        snapshot { std::move(snapshot) }
    {
    }

    TestSnapshotProvider::TestSnapshotProvider(market_data::Snapshot snapshot,
                                               const std::chrono::milliseconds snapshotDelay) noexcept :
         snapshot { std::move(snapshot) },
         snapshotDelay { snapshotDelay }
    {
    }

    [[nodiscard]]
    market_data::Snapshot TestSnapshotProvider::getSnapshot()
    {
        ++snapshotRequestedCount;
        if (snapshotDelay.count() > 0)
            std::this_thread::sleep_for(snapshotDelay);
        return snapshot;
    }

    [[nodiscard]]
    std::size_t TestSnapshotProvider::getSnapshotRequestedCount() const noexcept
    {
        return snapshotRequestedCount;
    }

    void TestSnapshotProvider::resetSnapshotRequestedCount() noexcept
    {
        snapshotRequestedCount = 0;
    }

    void TestSnapshotProvider::setSnapshot(market_data::Snapshot newSnapshot) noexcept
    {
        snapshot = std::move(newSnapshot);
    }

    void TestSnapshotProvider::setSnapshotDelay(const std::chrono::milliseconds delay) noexcept
    {
        snapshotDelay = delay;
    }
}