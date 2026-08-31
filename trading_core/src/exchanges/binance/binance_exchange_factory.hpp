/**============================================================================
Name        : binance_exchange_factory.hpp
Created on  : 31.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : binance_exchange_factory.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_BINANCE_EXCHANGE_FACTORY_HPP
#define TRADINGCOREMODULAREX_BINANCE_EXCHANGE_FACTORY_HPP

#include "exchange_factory.hpp"

#include "binance_execution_gateway.hpp"

namespace trading::exchanges::binance
{
    struct BinanceExchangeFactory final : IExchangeFactory
    {

        [[nodiscard]]
        std::unique_ptr<execution::IExecutionGateway>
        createExecutionGateway(const config::Config& config,
                               const common::RuntimeContext& runtimeContext) noexcept override;

        [[nodiscard]]
        std::unique_ptr<execution::IExecutionReportSource>
        createExecutionReportSource(const config::Config& config,
                                    concurrency::Queue<execution::ExecutionWorkItem>& executionQueue,
                                    const common::RuntimeContext& runtimeContext) noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::IMarketDataParser>
        createMarketDataParser(const config::Config& config,
                               const common::RuntimeContext& runtimeContext) noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::IMarketDataSource>
        createMarketDataSource(const config::Config& config,
                               const common::RuntimeContext& runtimeContext) noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::ISnapshotProvider>
        createSnapshotProvider(const config::Config& config,
                               const common::RuntimeContext& runtimeContext) noexcept override;
    };
}
#endif //TRADINGCOREMODULAREX_BINANCE_EXCHANGE_FACTORY_HPP
