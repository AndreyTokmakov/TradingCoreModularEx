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

#include <memory>

namespace trading::testing
{
    struct TestMocks
    {
        std::unique_ptr<execution::IExecutionGateway>      executionGateway { nullptr };
        std::unique_ptr<execution::IExecutionReportSource> executionReportSource { nullptr };
        std::unique_ptr<market_data::IMarketDataParser>    marketDataParser { nullptr };
        std::unique_ptr<market_data::IMarketDataSource>    marketDataSource { nullptr };
        std::unique_ptr<market_data::ISnapshotProvider>    snapshotProvider { nullptr };
    };

    class TestExchangeFactory final : public exchanges::IExchangeFactory
    {
    public:
        explicit TestExchangeFactory(std::unique_ptr<execution::IExecutionGateway> testExecutionGateway,
                                     std::unique_ptr<market_data::IMarketDataSource> testMarketDataSource = nullptr,
                                     std::unique_ptr<market_data::ISnapshotProvider> testSnapshotProvider = nullptr) noexcept;

        explicit TestExchangeFactory(TestMocks&& testMocks) noexcept;

        [[nodiscard]]
        std::unique_ptr<execution::IExecutionGateway>
        createExecutionGateway(const config::Config&) const noexcept override;

        [[nodiscard]]
        std::unique_ptr<execution::IExecutionReportSource>
        createExecutionReportSource(const config::Config&) const noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::IMarketDataParser>
        createMarketDataParser(const config::Config&) const noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::IMarketDataSource>
        createMarketDataSource(const config::Config&) const noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::ISnapshotProvider>
        createSnapshotProvider(const config::Config&) const noexcept override;

        [[nodiscard]]
        std::size_t createSnapshotProviderCountValue() const noexcept;

    private:

        mutable std::unique_ptr<execution::IExecutionGateway> executionGateway;
        mutable std::unique_ptr<execution::IExecutionReportSource> executionReportSource;
        mutable std::unique_ptr<market_data::IMarketDataParser> marketDataParser;
        mutable std::unique_ptr<market_data::IMarketDataSource> marketDataSource;
        mutable std::unique_ptr<market_data::ISnapshotProvider> snapshotProvider;

        mutable std::size_t createSnapshotProviderCount { 0 };
    };
}

#endif //TRADINGCOREMODULAREX_TEST_EXCHANGE_FACTORY_HPP
