/**============================================================================
Name        : exchange_factory.hpp
Created on  : 31.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : exchange_factory.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_EXCHANGE_FACTORY_HPP
#define TRADINGCOREMODULAREX_EXCHANGE_FACTORY_HPP

#include "config.hpp"
#include "queue.hpp"
#include "runtime_context.hpp"

#include "model/execution_work_item.hpp"

#include "interfaces/execution_gateway.hpp"
#include "interfaces/execution_report_source.hpp"
#include "interfaces/market_data_parser.hpp"
#include "interfaces/market_data_source.hpp"
#include "interfaces/snapshot_provider.hpp"

namespace trading::exchanges
{
    struct IExchangeFactory
    {
        virtual ~IExchangeFactory() = default;

        [[nodiscard]]
        virtual std::unique_ptr<execution::IExecutionGateway>
        createExecutionGateway(const config::Config& config,
                               const common::RuntimeContext& runtimeContext) noexcept = 0;

        [[nodiscard]]
        virtual std::unique_ptr<execution::IExecutionReportSource>
        createExecutionReportSource(const config::Config& config,
                                    concurrency::Queue<execution::ExecutionWorkItem>& executionQueue,
                                    const common::RuntimeContext& runtimeContext) noexcept = 0;

        [[nodiscard]]
        virtual std::unique_ptr<market_data::IMarketDataParser>
        createMarketDataParser(const config::Config& config,
                               const common::RuntimeContext& runtimeContext) noexcept = 0;

        [[nodiscard]]
        virtual std::unique_ptr<market_data::IMarketDataSource>
        createMarketDataSource(const config::Config& config,
                               const common::RuntimeContext& runtimeContext) noexcept = 0;

        [[nodiscard]]
        virtual std::unique_ptr<market_data::ISnapshotProvider>
        createSnapshotProvider(const config::Config& config,
                               const common::RuntimeContext& runtimeContext) noexcept = 0;
    };
}

#endif //TRADINGCOREMODULAREX_EXCHANGE_FACTORY_HPP
