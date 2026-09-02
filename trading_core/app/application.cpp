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
#include "exchange_factory_registry.hpp"

namespace trading::app
{
    using exchanges::ExchangeFactoryRegistry;

    Application::Application(const std::filesystem::path& configPath):
        config { config::loadConfig(configPath) },
        bookUpdateQueue {},
        strategyEventQueue {},
        recordingEventQueue {},
        executionQueue {},
        exchangeFactory {
            ExchangeFactoryRegistry::createFactory(exchanges::ExchangeType::Binance)
        },
        marketDataModule {
            config, bookUpdateQueue, *exchangeFactory
        },
        bookBuilderModule {
            config, bookUpdateQueue, strategyEventQueue, recordingEventQueue, *exchangeFactory
        },
        strategyModule {
            config.strategy, strategyEventQueue, executionQueue
        },
        executionModule {
            config, executionQueue, recordingEventQueue, *exchangeFactory
        },
        recordingModule {
            config.recording, recordingEventQueue
        },
        executionReportModule {
            config, executionQueue, *exchangeFactory
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
        executionReportModule.start();
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
        executionReportModule.stop();
        executionModule.stop();
        strategyModule.stop();

        running = false;
    }
}