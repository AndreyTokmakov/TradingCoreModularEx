/**============================================================================
Name        : trading_test_environment.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trading_test_environment.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TRADING_TEST_ENVIRONMENT_HPP
#define FINANCETECHNOLOGYPROJECTS_TRADING_TEST_ENVIRONMENT_HPP

#include "trading_test_configuration.hpp"
#include "book_builder.hpp"
#include "execution_report_handler.hpp"
#include "market_data_message_handler.hpp"
#include "market_event_handler.hpp"
#include "market_data_source.hpp"
#include "order_book.hpp"
#include "imbalance_strategy.hpp"
#include "strategy_executor.hpp"
#include "order_manager.hpp"
#include "position_manager.hpp"
#include "risk_manager.hpp"
#include "trade_recorder.hpp"

#include <memory>

namespace trading::testing
{
    class TradingTestEnvironment final
    {
    public:
        execution::OrderCreationResult createOrder(const execution::OrderRequest& request);

        const execution::Order *findOrder(OrderId orderId) const noexcept;

    public:
        TradingTestEnvironment(const std::shared_ptr<market_data::IMarketDataSource>& testMarketDataSource,
                               const std::shared_ptr<market_data::IMarketDataParser>& testMarketDataParser,
                               const std::shared_ptr<execution::IExecutionGateway>& testExecutionGateway,
                               const TradingTestConfiguration& configuration = {});

        TradingTestEnvironment(const TradingTestEnvironment&) = delete;
        TradingTestEnvironment& operator=(const TradingTestEnvironment&) = delete;

        TradingTestEnvironment(TradingTestEnvironment&&) = delete;
        TradingTestEnvironment& operator=(TradingTestEnvironment&&) = delete;

        std::shared_ptr<market_data::IMarketDataSource> marketDataSource;
        std::shared_ptr<market_data::IMarketDataParser> marketDataParser;
        std::shared_ptr<execution::IExecutionGateway> executionGateway;

        position::PositionManager positionManager;
        risk::RiskManager riskManager {};
        recording::TradeRecorder recorder;
        market_data::OrderBook orderBook;

        execution::OrderManager orderManager;
        execution::ExecutionReportHandler executionReportHandler;

        strategy::ImbalanceStrategy strategy;
        strategy::StrategyExecutor strategyExecutor;

        market_data::MarketEventHandler marketEventHandler;
        market_data::BookBuilder bookBuilder;
        market_data::MarketDataMessageHandler messageHandler;

        InstrumentId instrument;
        Quantity orderQuantity;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TRADING_TEST_ENVIRONMENT_HPP
