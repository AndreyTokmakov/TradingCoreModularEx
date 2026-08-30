/**============================================================================
Name        : test_market_data_source.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test market data source implementation.
============================================================================**/

#include "test_market_data_source.hpp"

#include <utility>

namespace trading::testing::stubs
{
    TestMarketDataSource::TestMarketDataSource(std::string endpoint,
                                               const common::RuntimeContext& runtimeContext) noexcept :
        endpoint { std::move(endpoint) },
        logger { runtimeContext.logger },
        metricsCollector { runtimeContext.metricsCollector }
    {
    }

    void TestMarketDataSource::start()
    {
        metrics = &metricsCollector.getThreadMetrics();

        logger.info("{} [{}] Connecting to '{}' ...",__PRETTY_FUNCTION__, __LINE__, endpoint);
        metrics->increment<metrics::MetricType::MarketDataReceived>();

        running = true;

        logger.info("{} [{}] Got MargetData", __PRETTY_FUNCTION__, __LINE__);
        messageHandler->onMessage("1,1000001,1640995200000,Buy,98765,100,1000");

        // TODO: Connect to endpoint.
        // TODO: Start receiving market data.
    }

    void TestMarketDataSource::stop()
    {
        running = false;

        // TODO: Close connection.
    }

    void TestMarketDataSource::setMessageHandler(market_data::IMarketDataMessageHandler& handler)
    {
        messageHandler = &handler;
    }
}