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
#include "json_config_loader.hpp"
#include "logger_factory.hpp"

namespace
{
    using trading::config::Config;
    using trading::config::Error;
    using trading::config::ExchangeConfig;
    using trading::config::JsonConfigLoader;

    [[nodiscard]]
    Config loadConfig(const std::filesystem::path& configPath)
    {
        if (const std::expected<Config, Error> result = JsonConfigLoader::load(configPath))
            return *result;
        throw std::runtime_error {"Failed to load configuration: " +configPath.string()};
    }

    [[nodiscard]]
    const ExchangeConfig& findExchange(const Config& config,
                                       const std::string_view name)
    {
        for (const ExchangeConfig& exchange : config.exchanges)
        {
            if (exchange.name == name)
                return exchange;
        }

        throw std::runtime_error {
            "Exchange configuration not found: " + std::string { name }
        };
    }
}

namespace trading::app
{
    Application::Application(const std::filesystem::path&) {
    }

    Application::~Application() {
        stop();
    }

    void Application::start() {
    }

    void Application::stop() {
    }
}