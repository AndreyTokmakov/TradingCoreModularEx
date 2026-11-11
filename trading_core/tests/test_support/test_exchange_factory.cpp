/**============================================================================
Name        : test_exchange_factory.hpp
Created on  : 06.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_exchange_factory.hpp
============================================================================**/

#include "test_exchange_factory.hpp"
#include "test_execution_gateway.hpp"
#include "test_market_data_parser.hpp"
#include "test_market_data_source.hpp"
#include "test_snapshot_provider.hpp"

#include <memory>

namespace trading::testing
{
    TestExchangeFactory::TestExchangeFactory(std::unique_ptr<execution::IOrderGateway> testOrderGateway,
                                             std::unique_ptr<market_data::IMarketDataSource> testMarketDataSource,
                                             std::unique_ptr<market_data::ISnapshotProvider> testSnapshotProvider) noexcept :
        orderGateway { std::move(testOrderGateway) },
        executionReportSource { nullptr },
        marketDataParser { std::make_unique<TestMarketDataParser>() },
        marketDataSource { std::move(testMarketDataSource) },
        snapshotProvider { std::move(testSnapshotProvider) }
    {
    }

    TestExchangeFactory::TestExchangeFactory(TestMocks&& testMocks) noexcept :
        orderGateway { std::move(testMocks.orderGateway) },
        executionReportSource { std::move(testMocks.executionReportSource) },
        marketDataParser { std::move(testMocks.marketDataParser) },
        marketDataSource { std::move(testMocks.marketDataSource) },
        snapshotProvider { std::move(testMocks.snapshotProvider) }
    {
    }

    [[nodiscard]]
    std::unique_ptr<execution::IOrderGateway>
    TestExchangeFactory::createOrderGateway(const config::Config&) const noexcept
    {
        return std::move(orderGateway);
    }

    [[nodiscard]]
    std::unique_ptr<execution::IExecutionReportSource>
    TestExchangeFactory::createExecutionReportSource(const config::Config&) const noexcept
    {
        return std::move(executionReportSource);
    }

    [[nodiscard]]
    std::unique_ptr<market_data::IMarketDataParser>
    TestExchangeFactory::createMarketDataParser(const config::Config&) const noexcept
    {
        return std::move(marketDataParser);
    }

    [[nodiscard]]
    std::unique_ptr<market_data::IMarketDataSource>
    TestExchangeFactory::createMarketDataSource(const config::Config&) const noexcept
    {
        return std::move(marketDataSource);
    }

    [[nodiscard]]
    std::unique_ptr<market_data::ISnapshotProvider>
    TestExchangeFactory::createSnapshotProvider(const config::Config&) const noexcept
    {
        ++createSnapshotProviderCount;
        return std::move(snapshotProvider);
    }

    [[nodiscard]]
    std::size_t TestExchangeFactory::createSnapshotProviderCountValue() const noexcept
    {
        return createSnapshotProviderCount;
    }
}
