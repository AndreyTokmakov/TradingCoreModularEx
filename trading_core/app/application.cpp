/**============================================================================
Name        : application.cpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : application.cpp
============================================================================**/

/*
    Application implementation.
    Application constructs and wires the market-data components.

    Application does not process market-data events itself. It only creates
    the components and establishes their relationships.
*/

#include "application.hpp"
#include "config_utils.hpp"
#include "logger_factory.hpp"
#include "binance_snapshot_provider.hpp"

namespace trading::app
{
    Application::Application(const std::filesystem::path& configPath):
        config { config::loadConfig(configPath) },
        runtimeContext {
            .logger = logging::LoggerFactory::createLogger({}, {}),
            .metricsCollector = metrics::MetricsCollector::getCollector()
        },
        bookUpdateQueue {},
        strategyEventQueue {},
        recordingEventQueue {},
        executionQueue {},
        marketDataModule {
            findExchange(config, "binance").marketDataEndpoint, bookUpdateQueue
        },
        bookBuilderModule {
            config, bookUpdateQueue, strategyEventQueue, recordingEventQueue
        },
        strategyModule {
            config.strategy, strategyEventQueue, executionQueue, runtimeContext
        },
        executionModule {
            config, executionQueue, recordingEventQueue, runtimeContext
        },
        recordingModule {
            config.recording, recordingEventQueue, runtimeContext
        }
    {
        // TODO
    }

    Application::~Application() {
        stop();
    }

    void Application::start()
    {
        if (running)
            return;

        running = true;

        strategyModule.start();
        bookBuilderModule.start();
        executionModule.start();
        recordingModule.start();

        marketDataModule.start();
    }

    void Application::stop()
    {
        if (!running)
            return;

        marketDataModule.stop();

        recordingModule.stop();
        bookBuilderModule.stop();
        executionModule.stop();
        strategyModule.stop();

        running = false;
    }
}