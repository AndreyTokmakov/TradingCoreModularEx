/**============================================================================
Name        : binance_exchange_factory.cpp
Created on  : 31.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : binance_exchange_factory.cpp
============================================================================**/

#include "config_utils.hpp"
#include "binance_exchange_factory.hpp"
#include "binance_execution_gateway.hpp"
#include "binance_execution_report_source.hpp"
#include "binance_market_data_parser.hpp"
#include "binance_market_data_source.hpp"
#include "binance_snapshot_provider.hpp"

namespace trading::exchanges::binance
{
    constexpr std::string_view ExchangeName = "binance";

    [[nodiscard]]
    std::unique_ptr<execution::IExecutionGateway>
    BinanceExchangeFactory::createExecutionGateway(const config::Config& config,
                                                   const common::RuntimeContext&) const noexcept
    {
        return std::make_unique<BinanceExecutionGateway>(
            findExchange(config, ExchangeName).executionEndpoint
        );
    }

    [[nodiscard]]
    std::unique_ptr<execution::IExecutionReportSource>
    BinanceExchangeFactory::createExecutionReportSource(const config::Config& config,
                                                        concurrency::Queue<execution::ExecutionWorkItem>& executionQueue,
                                                        const common::RuntimeContext&) const noexcept
    {
        return std::make_unique<BinanceExecutionReportSource>(
            findExchange(config, ExchangeName).executionEndpoint,
            executionQueue
        );
    }

    [[nodiscard]]
    std::unique_ptr<market_data::IMarketDataParser>
    BinanceExchangeFactory::createMarketDataParser(const config::Config&,
                                                   const common::RuntimeContext&) const noexcept
    {
        return std::make_unique<BinanceMarketDataParser>();
    }

    [[nodiscard]]
    std::unique_ptr<market_data::IMarketDataSource>
    BinanceExchangeFactory::createMarketDataSource(const config::Config& config,
                                                   const common::RuntimeContext&) const noexcept
    {
        return std::make_unique<BinanceMarketDataSource>(
            config::findExchange(config, ExchangeName).marketDataEndpoint
        );
    }

    [[nodiscard]]
    std::unique_ptr<market_data::ISnapshotProvider>
    BinanceExchangeFactory::createSnapshotProvider(const config::Config& config,
                                                   const common::RuntimeContext&) const noexcept
    {
        return std::make_unique<BinanceSnapshotProvider>(
            config::findExchange(config, ExchangeName).marketDataEndpoint
        );
    }
}