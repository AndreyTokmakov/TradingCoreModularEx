/**============================================================================
Name        : test_snapshot_provider.hpp
Created on  : 25.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_snapshot_provider.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_TEST_SNAPSHOT_PROVIDER_HPP
#define TRADINGCOREMODULAREX_TEST_SNAPSHOT_PROVIDER_HPP

#include <chrono>
#include "market_data/interfaces/snapshot_provider.hpp"

#include "test_support/debug_helpers.hpp"
using namespace  trading::testing;


namespace trading::testing
{
    class TestSnapshotProvider final : public market_data::ISnapshotProvider
    {
    public:
        explicit TestSnapshotProvider(market_data::Snapshot snapshot) noexcept;

        explicit TestSnapshotProvider(market_data::Snapshot snapshot,
                                      std::chrono::milliseconds snapshotDelay) noexcept;

        [[nodiscard]]
        market_data::Snapshot getSnapshot() override;

        [[nodiscard]]
        std::size_t getSnapshotRequestedCount() const noexcept;

        void resetSnapshotRequestedCount() noexcept;
        void setSnapshot(market_data::Snapshot newSnapshot) noexcept;
        void setSnapshotDelay(std::chrono::milliseconds delay) noexcept;

    private:
        market_data::Snapshot snapshot;
        std::chrono::milliseconds snapshotDelay {};
        std::size_t snapshotRequestedCount { 0 };
    };

}


#endif //TRADINGCOREMODULAREX_TEST_SNAPSHOT_PROVIDER_HPP
