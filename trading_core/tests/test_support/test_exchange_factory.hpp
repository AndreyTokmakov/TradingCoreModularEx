/**============================================================================
Name        : test_exchange_factory.hpp
Created on  : 06.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_exchange_factory.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_TEST_EXCHANGE_FACTORY_HPP
#define TRADINGCOREMODULAREX_TEST_EXCHANGE_FACTORY_HPP

#include "exchanges/exchange_factory.hpp"
#include "test_market_data_parser.hpp"
#include "test_market_data_source.hpp"
#include "test_snapshot_provider.hpp"

#include <memory>

namespace trading::testing
{
    class TestExchangeFactory final : public exchanges::IExchangeFactory
    {
    public:
        explicit TestExchangeFactory(std::unique_ptr<TestMarketDataSource> testMarketDataSource,
                                     std::unique_ptr<TestSnapshotProvider> testSnapshotProvider = nullptr) noexcept :
            marketDataSource { std::move(testMarketDataSource) },
            snapshotProvider { std::move(testSnapshotProvider) }
        {
        }

        [[nodiscard]]
        std::unique_ptr<execution::IExecutionGateway>
        createExecutionGateway(const config::Config&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<execution::IExecutionReportSource>
        createExecutionReportSource(const config::Config&,
                                    concurrency::Queue<execution::ExecutionWorkItem>&) const noexcept override
        {
            return nullptr;
        }

        [[nodiscard]]
        std::unique_ptr<market_data::IMarketDataParser>
        createMarketDataParser(const config::Config&) const noexcept override
        {
            return std::make_unique<TestMarketDataParser>();
        }

        [[nodiscard]]
        std::unique_ptr<market_data::IMarketDataSource>
        createMarketDataSource(const config::Config&) const noexcept override
        {
            return std::move(marketDataSource);
        }

        [[nodiscard]]
        std::unique_ptr<market_data::ISnapshotProvider>
        createSnapshotProvider(const config::Config&) const noexcept override
        {
            return std::move(snapshotProvider);
        }

    private:
        mutable std::unique_ptr<TestMarketDataSource> marketDataSource;
        mutable std::unique_ptr<TestSnapshotProvider> snapshotProvider;
    };
}

#endif //TRADINGCOREMODULAREX_TEST_EXCHANGE_FACTORY_HPP
