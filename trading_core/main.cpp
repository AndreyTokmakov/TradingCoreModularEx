/**============================================================================
Name        : main.cpp
Created on  : 
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : 
============================================================================**/


#include <iostream>
#include <filesystem>
#include <vector>
#include <string_view>

#include "app/application.hpp"

void price_test();
void order_book_test();
void order_manager_test();
void book_builder_test();

void market_data_module_test();
void book_builder_module_test();
void marketdata_bookbuilder_strategy_integrataion();
void strategy_module_test();

void market_event_handler_test();
void pnl_calculator_test();
void risk_manager_test();
void trade_recorder_test();
void position_test();
void position_manager_test();
void imbalance_strategy_test();
void strategy_executor_test();
void json_config_loader_test();

void trading_integration_test();
void trading_inbound_integration_test();


// TODO: - Improvements
//  - Заменить std::map<K,V> для PriceLevel-ов ---> DenseMap

// TODO: Metics
//  - Put Collection on CPU-5
//  - Grafana / Prometheus ?

// TODO:
//  - vector of MarketDataModule, BookBuilderModule, ExecutionReportModule ... per Exchange ??

// TODO: Metrics
//  • Market Data:
//       - messages received
//       - messages parsed
//       - parse errors
//       - book updates
//       - book update errors
//       - market events
//  • Strategy:
//       - events evaluated
//       - Buy signals
//       - Sell signals
//       - orders generated
//  • Risk:
//       - risk checks
//       - risk rejections
//  • Execution:
//       - order requests
//       - orders created
//       - orders rejected
//       - execution reports
//       - trades
//       - cancelled
//       - rejected
//  • Order Book:
//       - snapshots applied
//       - updates applied
//       - sequence gaps

// TODO:
//  ---- > where to use PnLCalculator ??


/**  Сейчас есть в PositionManager есть applyExecution и applyTrade
 *   разобраться какое нужен и какой оставить
 *   попроавить тесты

bool PositionManager::applyExecution(const execution::ExecutionReport& report)
    {
        if (report.execType != ExecType::Trade || report.quantity.isZero())
            return false;

        auto [it, inserted] = positions.try_emplace(report.instrument, report.instrument);

        Position& position = it->second;
        position.applyTrade(report.side, report.price, report.quantity);

        return true;
    }

    bool PositionManager::applyTrade(const InstrumentId instrument,
                                     const Side side,
                                     const Price price,
                                     const Quantity quantity)
    {
        if (quantity.isZero())
            return false;

        auto [it, inserted] = positions.try_emplace(instrument, instrument);

        Position& position = it->second;
        position.applyTrade(side, price, quantity);

        return true;
    }
**/


namespace
{
    [[maybe_unused]]
    void runApp(const std::vector<std::string_view>& parameters)
    {
        const std::filesystem::path configPath = parameters.empty() ?  "config.json" : parameters.front();
        try
        {
            trading::app::Application application { configPath };
            application.start();

            // Application lifetime will be controlled here.
            // TODO: Add signal handling / graceful shutdown.
        }
        catch (const std::exception& error)
        {
            std::cerr << "Application startup failed: "<< error.what() << '\n';
        }
    }

    [[maybe_unused]]
    void runTests(const std::vector<std::string_view>& )
    {
        price_test();

        // market_event_handler_test();
        // strategy_executor_test();

        json_config_loader_test();
        trade_recorder_test();
        pnl_calculator_test();
        risk_manager_test();
        position_test();
        position_manager_test();
        order_book_test();
        order_manager_test();
        book_builder_test();
        imbalance_strategy_test();

        market_data_module_test();
        book_builder_module_test();
        strategy_module_test();
    }
}

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> parameters(argv + 1, argv + argc);

    // runApp(parameters);
    // runTests(parameters);
    marketdata_bookbuilder_strategy_integrataion();

    return EXIT_SUCCESS;
}
