/**============================================================================
Name        : snapshot_provider.hpp
Created on  : 27.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market-data snapshot provider interface.
============================================================================**/

/*
    ISnapshotProvider defines the boundary used by BookBuilderWorker to obtain
    the initial order-book Snapshot.

    A Snapshot represents a complete order-book state and is used to initialize
    the OrderBook before incremental BookUpdate events are applied.

    Data Flow:

        ISnapshotProvider
               |
               | Snapshot
               v
        BookBuilderWorker
               |
               | applySnapshot()
               v
           BookBuilder
               |
               v
           OrderBook

    The provider is responsible only for obtaining a Snapshot.

    It does not:

        - apply the Snapshot to OrderBook;
        - process incremental BookUpdate events;
        - know about BookBuilder;
        - publish MarketEvent;
        - manage market-data queues.

    Exchange-specific implementations belong to the exchange integration layer,
    for example:

        exchanges::binance::BinanceSnapshotProvider
*/

#ifndef FINANCETECHNOLOGYPROJECTS_SNAPSHOT_PROVIDER_HPP
#define FINANCETECHNOLOGYPROJECTS_SNAPSHOT_PROVIDER_HPP

#include "model/snapshot.hpp"

namespace trading::market_data
{
    struct ISnapshotProvider
    {
        virtual ~ISnapshotProvider() = default;

        [[nodiscard]]
        virtual Snapshot getSnapshot() = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_SNAPSHOT_PROVIDER_HPP