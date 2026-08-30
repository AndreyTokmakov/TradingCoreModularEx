/**============================================================================
Name        : test_market_data_source.hpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test market data source.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TEST_MARKET_DATA_SOURCE_HPP
#define FINANCETECHNOLOGYPROJECTS_TEST_MARKET_DATA_SOURCE_HPP

#include "market_data_message_handler.hpp"
#include "interfaces/market_data_source.hpp"

#include "recorder.hpp"
#include "runtime_context.hpp"

namespace trading::testing::stubs
{
    class TestMarketDataSource final : public market_data::IMarketDataSource
    {
    public:
        explicit TestMarketDataSource(std::string endpoint,
                                      const common::RuntimeContext& runtimeContext) noexcept;

        void start() override;
        void stop() override;

        void setMessageHandler(market_data::IMarketDataMessageHandler& handler) override;

    private:
        std::string endpoint;
        market_data::IMarketDataMessageHandler* messageHandler { nullptr };
        bool running { false };

        logging::ILogger& logger;
        metrics::MetricsCollector& metricsCollector;
        metrics::Metrics* metrics { nullptr };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TEST_MARKET_DATA_SOURCE_HPP