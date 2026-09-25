/**============================================================================
Name        : test_exchange_factory.hpp
Created on  : 06.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_exchange_factory.hpp
============================================================================**/

#include "test_exchange_factory.hpp"

#include <memory>

namespace trading::testing
{
     TestExchangeFactory::TestExchangeFactory(std::unique_ptr<TestMarketDataSource> testMarketDataSource,
                                 std::unique_ptr<TestSnapshotProvider> testSnapshotProvider ) noexcept :
        marketDataSource { std::move(testMarketDataSource) },
        snapshotProvider { std::move(testSnapshotProvider) }
    {
    }

    [[nodiscard]]
    std::unique_ptr<execution::IExecutionGateway>
    TestExchangeFactory::createExecutionGateway(const config::Config&) const noexcept
    {
        return nullptr;
    }

    [[nodiscard]]
    std::unique_ptr<execution::IExecutionReportSource>
    TestExchangeFactory::createExecutionReportSource(const config::Config&,
                                concurrency::Queue<execution::ExecutionWorkItem>&) const noexcept
    {
        return nullptr;
    }

    [[nodiscard]]
    std::unique_ptr<market_data::IMarketDataParser>
    TestExchangeFactory::createMarketDataParser(const config::Config&) const noexcept
    {
        return std::make_unique<TestMarketDataParser>();
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
        return std::move(snapshotProvider);
    }
}
