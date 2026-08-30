/**============================================================================
Name        : trading_test_environment.сpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trading_test_environment.сpp
============================================================================**/

#include "trading_test_environment.hpp"
#include "test_market_data_parser.hpp"
#include "test_market_data_source.hpp"
#include "test_execution_gateway.hpp"

namespace trading::testing
{
    TradingTestEnvironment::TradingTestEnvironment(const std::shared_ptr<market_data::IMarketDataSource>& testMarketDataSource,
                               const std::shared_ptr<market_data::IMarketDataParser>& testMarketDataParser,
                               const std::shared_ptr<execution::IExecutionGateway>& testExecutionGateway,
                               const TradingTestConfiguration& configuration):
        marketDataSource { testMarketDataSource },
        marketDataParser { testMarketDataParser },
        executionGateway { testExecutionGateway },
        positionManager {},
        riskManager  {},
        recorder {},
        orderBook {},
        orderManager {
            *executionGateway, riskManager, positionManager
        },
        executionReportHandler {
            orderManager, positionManager, recorder
        },
        strategy {
            configuration.thresholdNumerator,
            configuration.thresholdDenominator
        },
        strategyExecutor {
            orderManager, configuration.orderQuantity
        },
        marketEventHandler {
            strategy, strategyExecutor, recorder
        },
        bookBuilder {
            configuration.instrument, orderBook, marketEventHandler
        },
        messageHandler {
            *marketDataParser, bookBuilder
        },
        instrument {
            configuration.instrument
        },
        orderQuantity {
            configuration.orderQuantity
        }
    {
        marketDataSource->setMessageHandler(messageHandler);
    }

    execution::OrderCreationResult
    TradingTestEnvironment::createOrder(const execution::OrderRequest& request)
    {
        return orderManager.createOrder(request);
    }

    const execution::Order *TradingTestEnvironment::findOrder(const OrderId orderId) const noexcept
    {
        return orderManager.find(orderId);
    }

}
