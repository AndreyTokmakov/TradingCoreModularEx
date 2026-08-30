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


void order_book_test();
void order_manager_test();
void market_event_handler_test();
void execution_report_handler_test();
void book_builder_test();
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
void e2eTests();

// TODO: - Worker --> Modules
//  - Rename files
//  - Add new description for Apps

// TODO: - Improvements
//  - Заменить std::map<K,V> для PriceLevel-ов ---> DenseMap

// TODO: Metics
//  - Put Collection on CPU-5
//  - Grafana

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
        order_book_test();
        order_manager_test();
        market_event_handler_test();
        execution_report_handler_test();
        book_builder_test();
        pnl_calculator_test();
        risk_manager_test();
        trade_recorder_test();
        position_test();
        position_manager_test();
        imbalance_strategy_test();
        strategy_executor_test();
        json_config_loader_test();
    }

    [[maybe_unused]]
    void runIntegrationTests(const std::vector<std::string_view>&)
    {
        // trading_integration_test();
        // trading_inbound_integration_test();
    }
}

// TODO:
//  - Нужно ли при применении Shanpshot-а вообще пробовать формировать MarketEvent ??

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> parameters(argv + 1, argv + argc);

    // runApp(parameters);
    // runTests(parameters);
    // runIntegrationTests(parameters);
    // e2eTests();

    return EXIT_SUCCESS;
}
