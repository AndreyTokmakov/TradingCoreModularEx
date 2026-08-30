/**============================================================================
Name        : application.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : application.hpp
============================================================================**/

/*
    Application is the composition root of the trading system.

    Application constructs application components, connects their dependencies
    and controls the application lifecycle.

    Responsibilities:

        - construct application components;
        - establish dependencies between components;
        - configure the market-data pipeline;
        - control application lifecycle.

    Application does not implement trading logic. Domain responsibilities
    remain inside the corresponding modules.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP
#define FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP

#include "config.hpp"
#include "condition_variable_queue.hpp"

#include "execution_module.hpp"
#include "execution_report_module.hpp"
#include "book_builder_module.hpp"
#include "market_data_module.hpp"
#include "recording_module.hpp"
#include "strategy_module.hpp"


namespace trading::app
{
    class Application final
    {
    public:
        explicit Application(const std::filesystem::path& configPath);
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        void start();
        void stop();

    private:

        config::Config config;
        common::RuntimeContext runtimeContext;

        concurrency::ConditionVariableQueue<market_data::BookUpdates> bookUpdateQueue;
        concurrency::ConditionVariableQueue<market_data::MarketEvent> strategyEventQueue;
        concurrency::ConditionVariableQueue<recording::RecordingEvent> recordingEventQueue;
        concurrency::ConditionVariableQueue<execution::ExecutionWorkItem> executionQueue;

        market_data::MarketDataModule marketDataModule;
        market_data::BookBuilderModule bookBuilderModule;
        strategy::StrategyModule  strategyModule;
        execution::ExecutionModule executionModule;
        recording::RecordingModule recordingModule;
        execution::ExecutionReportModule  executionReportModule;

        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP