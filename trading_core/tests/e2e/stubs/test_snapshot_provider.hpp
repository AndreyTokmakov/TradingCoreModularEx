/**============================================================================
Name        : test_snapshot_provider.hpp
Created on  : 27.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test implementation of the market-data snapshot provider.
============================================================================**/


#ifndef FINANCETECHNOLOGYPROJECTS_TEST_SNAPSHOT_PROVIDER_HPP
#define FINANCETECHNOLOGYPROJECTS_TEST_SNAPSHOT_PROVIDER_HPP

#include "interfaces/snapshot_provider.hpp"

#include <functional>
#include <string>

namespace trading::testing::stubs
{
    class TestSnapshotProvider final : public market_data::ISnapshotProvider
    {
    public:
        using FetchHandler = std::function<market_data::Snapshot()>;

        explicit TestSnapshotProvider(std::string endpoint) noexcept;

        TestSnapshotProvider(std::string endpoint,
                                 FetchHandler fetchHandler) noexcept;

        void setFetchHandler(FetchHandler fetchHandler) noexcept;

        [[nodiscard]]
        market_data::Snapshot getSnapshot() override;

    private:
        std::string endpoint;
        FetchHandler fetchHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TEST_SNAPSHOT_PROVIDER_HPP