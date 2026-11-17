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

#include "exchanges/exchange_factory.hpp"
#include "exchanges/exchange_type.hpp"
#include "network/network_runtime.hpp"

namespace trading::exchanges::binance
{
    struct BinanceExchangeFactory final : IExchangeFactory
    {
        explicit BinanceExchangeFactory(networking::NetworkRuntime &network_runtime):
            networkRuntime { network_runtime }
        {
        }

        [[nodiscard]]
        std::unique_ptr<order_management::IOrderGateway>
        createOrderGateway(const config::Config& config) const noexcept override;

        [[nodiscard]]
        std::unique_ptr<execution::IExecutionReportSource>
        createExecutionReportSource(const config::Config& config) const noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::IMarketDataParser>
        createMarketDataParser(const config::Config& config) const noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::IMarketDataSource>
        createMarketDataSource(const config::Config& config) const noexcept override;

        [[nodiscard]]
        std::unique_ptr<market_data::ISnapshotProvider>
        createSnapshotProvider(const config::Config& config) const noexcept override;

    private:

        networking::NetworkRuntime& networkRuntime;
    };
}
#endif //TRADINGCOREMODULAREX_BINANCE_EXCHANGE_FACTORY_HPP
