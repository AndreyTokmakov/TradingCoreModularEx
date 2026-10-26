/**============================================================================
Name        : binance_snapshot_provider.hpp
Created on  : 27.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance implementation of the market-data snapshot provider.
============================================================================**/

/*
    BinanceSnapshotProvider obtains an initial order-book Snapshot from Binance.

    The provider belongs to the Binance exchange integration layer and converts
    Binance-specific snapshot data into the normalized market_data::Snapshot
    representation.

    Data Flow:

        Binance REST API
               |
               | Binance snapshot
               v
        BinanceSnapshotProvider
               |
               | Snapshot
               v
        BookBuilderWorker
               |
               v
        BookBuilder
               |
               v
        OrderBook


    Network transport is injected through FetchHandler.

    This keeps BinanceSnapshotProvider independent of a particular HTTP client
    or networking implementation and makes the component easy to test.

    BinanceSnapshotProvider does not:

        - apply the snapshot to OrderBook;
        - process incremental market-data updates;
        - manage the market-data queue;
        - know about BookBuilder;
        - publish MarketEvent.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_SNAPSHOT_PROVIDER_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_SNAPSHOT_PROVIDER_HPP

#include "market_data/model/snapshot.hpp"
#include "market_data/interfaces/snapshot_provider.hpp"
#include "binance_market_data_parser.hpp"

#include "network/rest_client.hpp"
#include <string>

namespace trading::exchanges::binance
{
    class BinanceSnapshotProvider final : public market_data::ISnapshotProvider
    {
    public:
        explicit BinanceSnapshotProvider(std::string endpoint) noexcept;

        [[nodiscard]]
        std::expected<market_data::Snapshot, common::ErrorCode>
        getSnapshot(std::string_view symbol, uint32_t depth = 1'000) override;

    private:
        std::string endpoint;

        boost::asio::io_context ioContext;
        boost::asio::ssl::context sslContext;
        network::RestClient restClient;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_SNAPSHOT_PROVIDER_HPP