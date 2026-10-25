/**============================================================================
Name        : binance_snapshot_provider.cpp
Created on  : 27.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market-data snapshot provider implementation.
============================================================================**/

#include "binance_snapshot_provider.hpp"
#include "utilities/utilities.hpp"

#include <iostream>
#include <utility>
#include <format>

#include <simdjson.h>

namespace
{
    using namespace trading::utilities;

    constexpr std::string_view LastUpdateId { "lastUpdateId" };
    constexpr std::string_view Bids { "bids" };
    constexpr std::string_view Asks { "asks" };

    bool parseDepthSnapshot(const std::string_view message,
                             trading::market_data::Snapshot& snapshot) noexcept
    {
        simdjson::dom::parser parser;
        const simdjson::dom::element document = parser.parse(message.data(), message.size());

        snapshot.sequence = static_cast<trading::SequenceNumber>(document[LastUpdateId].get_uint64());
        {
            const auto& bids = document[Bids].get_array();
            snapshot.bids.reserve(bids.size());
            for (const auto& level : bids)
            {
                const simdjson::dom::array values = level.get_array();
                const std::string_view price = values.at(0).get_string();
                const std::string_view quantity = values.at(1).get_string();
                snapshot.bids.emplace_back(parsePrice(price), parseQuantity(quantity));
            }
        }
        {
            const auto& asks = document[Asks].get_array();
            snapshot.asks.reserve(asks.size());
            for (const auto& level : asks)
            {
                const simdjson::dom::array values = level.get_array();
                const std::string_view price = values.at(0).get_string();
                const std::string_view quantity = values.at(1).get_string();
                snapshot.asks.emplace_back(parsePrice(price), parseQuantity(quantity));
            }
        }
        return true;
    }
}

namespace trading::exchanges::binance
{
    BinanceSnapshotProvider::BinanceSnapshotProvider(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) },
        ioContext { 1 },
        sslContext { boost::asio::ssl::context::tls_client },
        restClient { ioContext, sslContext }
    {
    }

    market_data::Snapshot BinanceSnapshotProvider::getSnapshot(const std::string_view symbol, uint32_t depth)
    {
        if (ioContext.stopped()) {
            ioContext.restart();
        }

        network::HttpRequest request;
        request.host = endpoint;
        request.target = std::format("/api/v3/depth?symbol={}&limit={}", urlEncode(symbol), depth);
        request.method = boost::beast::http::verb::get;

        market_data::Snapshot snapshot;
        restClient.asyncRequest(request,[&snapshot](network::RestClient::Response response) mutable
        {
            if (!response) {
                std::cerr << response.error().message << std::endl;
                return;
            }
            if (!parseDepthSnapshot(response->body, snapshot)) {
                std::cerr << "Parse depth snapshot failed" << std::endl;
            }
        });

        const auto _ = ioContext.run();
        return snapshot;
    }
}