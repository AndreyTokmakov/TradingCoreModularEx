/**============================================================================
Name        : binance_exchange_factory.cpp
Created on  : 31.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : binance_exchange_factory.cpp
============================================================================**/

#include "config/config_utils.hpp"
#include "binance_exchange_factory.hpp"
#include "binance_order_gateway.hpp"
#include "binance_execution_report_source.hpp"
#include "binance_market_data_parser.hpp"
#include "binance_market_data_source.hpp"
#include "binance_snapshot_provider.hpp"

namespace trading::exchanges::binance
{
    constexpr std::string_view ExchangeName = "binance";

    [[nodiscard]]
    std::unique_ptr<order_management::IOrderGateway>
    BinanceExchangeFactory::createOrderGateway(const config::Config& config) const noexcept
    {
        const config::ExchangeConfig& cfgBinance = findExchange(config, ExchangeName);
        return std::make_unique<BinanceOrderGateway>(
            cfgBinance.wsUserDataStreamEndpoint,
            cfgBinance.apiKey,
            cfgBinance.apiSecret
        );
    }

    [[nodiscard]]
    std::unique_ptr<execution::IExecutionReportSource>
    BinanceExchangeFactory::createExecutionReportSource(const config::Config& config) const noexcept
    {
        const config::ExchangeConfig& cfgBinance = findExchange(config, ExchangeName);
        return std::make_unique<BinanceExecutionReportSource>(
            cfgBinance.restApiEndpoint,
            cfgBinance.apiKey,
            cfgBinance.apiSecret
        );
    }

    [[nodiscard]]
    std::unique_ptr<market_data::IMarketDataParser>
    BinanceExchangeFactory::createMarketDataParser(const config::Config&) const noexcept
    {
        return std::make_unique<BinanceMarketDataParser>();
    }

    [[nodiscard]]
    std::unique_ptr<market_data::IMarketDataSource>
    BinanceExchangeFactory::createMarketDataSource(const config::Config& config) const noexcept
    {
        return std::make_unique<BinanceMarketDataSource>(
            config::findExchange(config, ExchangeName).wsMarketDataEndpoint
        );
    }

    [[nodiscard]]
    std::unique_ptr<market_data::ISnapshotProvider>
    BinanceExchangeFactory::createSnapshotProvider(const config::Config& config) const noexcept
    {
        return std::make_unique<BinanceSnapshotProvider>(
            config::findExchange(config, ExchangeName).restApiEndpoint
        );
    }
}