/**============================================================================
Name        : binance_snapshot_provider.cpp
Created on  : 27.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market-data snapshot provider implementation.
============================================================================**/

#include "binance_snapshot_provider.hpp"

#include <utility>

namespace trading::exchanges::binance
{
    BinanceSnapshotProvider::BinanceSnapshotProvider(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) }
    {
    }

    BinanceSnapshotProvider::BinanceSnapshotProvider(std::string endpoint,
                                                     FetchHandler fetchHandler) noexcept :
        endpoint { std::move(endpoint) },
        fetchHandler { std::move(fetchHandler) }
    {
    }

    void BinanceSnapshotProvider::setFetchHandler(FetchHandler handler) noexcept
    {
        fetchHandler = std::move(handler);
    }

    market_data::Snapshot BinanceSnapshotProvider::getSnapshot()
    {
        if (!fetchHandler)
            return {};

        return fetchHandler();
    }
}