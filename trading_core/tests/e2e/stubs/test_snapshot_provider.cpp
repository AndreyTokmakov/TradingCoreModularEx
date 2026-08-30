/**============================================================================
Name        : test_snapshot_provider.cpp
Created on  : 27.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test market-data snapshot provider implementation.
============================================================================**/

#include "test_snapshot_provider.hpp"

#include <utility>
#include <iostream>

namespace trading::testing::stubs
{
    TestSnapshotProvider::TestSnapshotProvider(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) }
    {
    }

    TestSnapshotProvider::TestSnapshotProvider(std::string endpoint,
                                                     FetchHandler fetchHandler) noexcept :
        endpoint { std::move(endpoint) },
        fetchHandler { std::move(fetchHandler) }
    {
    }

    void TestSnapshotProvider::setFetchHandler(FetchHandler handler) noexcept
    {
        fetchHandler = std::move(handler);
    }

    market_data::Snapshot TestSnapshotProvider::getSnapshot()
    {
        std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "] " << std::endl;
        if (!fetchHandler)
            return {};

        std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "] " << std::endl;
        return fetchHandler();
    }
}