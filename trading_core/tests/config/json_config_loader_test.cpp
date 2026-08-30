/**============================================================================
Name        : json_config_loader_test.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Unit tests for JSON configuration loader.
============================================================================**/

#include "json_config_loader.hpp"
#include "test_support/testing.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using trading::InstrumentId;
using trading::Price;
using trading::Quantity;

using trading::config::Config;
using trading::config::Error;
using trading::config::JsonConfigLoader;

namespace
{
    using testing::Assert;


    std::filesystem::path createConfigFile(const std::string& content)
    {
        const std::filesystem::path path = std::filesystem::temp_directory_path() / "trading_config_test.json";

        std::ofstream file { path };
        Assert(file.is_open(), "test configuration file must be created");

        file << content;
        file.close();

        return path;
    }


    void removeConfigFile(const std::filesystem::path& path)
    {
        std::error_code error;
        std::filesystem::remove(path, error);
    }


    void testLoadValidMinimalConfiguration()
    {
        const std::filesystem::path path = createConfigFile(R"({
            "instrument": 42,
            "strategy": {
                "orderQuantity": 100000000,
                "thresholdNumerator": 7,
                "thresholdDenominator": 10
            }
        })");

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        removeConfigFile(path);
        Assert(result.has_value(), "valid configuration must be loaded");

        const Config& config = *result;

        Assert(config.instrument == InstrumentId { 42 },"invalid instrument");
        Assert(config.strategy.orderQuantity == Quantity { 100000000 },"invalid strategy order quantity");
        Assert(config.strategy.thresholdNumerator == 7,"invalid strategy threshold numerator");
        Assert(config.strategy.thresholdDenominator == 10,"invalid strategy threshold denominator");
    }


    void testLoadFullConfiguration()
    {
        const std::filesystem::path path = createConfigFile(R"({
            "instrument": 42,
            "strategy": {
                "orderQuantity": 100000000,
                "thresholdNumerator": 7,
                "thresholdDenominator": 10
            },
            "risk": {
                "maxOrderQuantity": 500000000,
                "maxPositionQuantity": 1000000000,
                "maxNotional": 2500000000000
            },
            "exchanges": [
                {
                    "name": "binance",
                    "marketDataEndpoint": "wss://market-data.example",
                    "executionEndpoint": "https://execution.example"
                },
                {
                    "name": "test-exchange",
                    "marketDataEndpoint": "wss://test-market.example",
                    "executionEndpoint": "https://test-execution.example"
                }
            ],
            "recording": {
                "enabled": false,
                "directory": "/tmp/trading-records"
            }
        })");

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        removeConfigFile(path);
        Assert(result.has_value(), "valid full configuration must be loaded");

        const Config& config = *result;
        Assert(config.instrument == InstrumentId { 42 },"invalid instrument");
        Assert(config.strategy.orderQuantity == Quantity { 100000000 },"invalid strategy order quantity");
        Assert(config.strategy.thresholdNumerator == 7,"invalid strategy threshold numerator");
        Assert(config.strategy.thresholdDenominator == 10,"invalid strategy threshold denominator");
        Assert(config.riskLimits.maxOrderQuantity == Quantity { 500000000 },"invalid maximum order quantity");
        Assert(config.riskLimits.maxPositionQuantity == Quantity { 1000000000 },"invalid maximum position quantity");
        Assert(config.riskLimits.maxNotional == Price { 2500000000000 },"invalid maximum notional");
        Assert(config.exchanges.size() == 2,"invalid exchange count");
        Assert(config.exchanges[0].name == "binance","invalid first exchange name");
        Assert(config.exchanges[0].marketDataEndpoint == "wss://market-data.example","invalid first market data endpoint");
        Assert(config.exchanges[0].executionEndpoint == "https://execution.example","invalid first execution endpoint");
        Assert(config.exchanges[1].name == "test-exchange","invalid second exchange name");
        Assert(!config.recording.enabled,"recording must be disabled");
        Assert(config.recording.directory == "/tmp/trading-records","invalid recording directory");
    }

    void testLoadFailsWhenFileDoesNotExist()
    {
        const std::filesystem::path path =
            std::filesystem::temp_directory_path() / "non_existing_trading_config.json";

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        Assert(!result.has_value(), "missing configuration file must fail");
        Assert(result.error() == Error::FileOpenFailed,"invalid error for missing configuration file");
    }

    void testLoadFailsForInvalidJson()
    {
        const std::filesystem::path path = createConfigFile(R"({
            "instrument": 42,
            "strategy": {
                "orderQuantity": 100000000,
                "thresholdNumerator": 7,
                "thresholdDenominator": 10
        })");

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        removeConfigFile(path);

        Assert(!result.has_value(), "invalid JSON must fail");
        Assert(result.error() == Error::InvalidFormat,"invalid error for malformed JSON");
    }


    void testLoadFailsForInvalidJsonType()
    {
        const std::filesystem::path path = createConfigFile(R"({
            "instrument": "BTCUSDT",
            "strategy": {
                "orderQuantity": 100000000,
                "thresholdNumerator": 7,
                "thresholdDenominator": 10
            }
        })");

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        removeConfigFile(path);

        Assert(!result.has_value(), "invalid JSON type must fail");
        Assert(result.error() == Error::InvalidConfiguration,"invalid error for invalid JSON type");
    }


    void testLoadFailsForZeroThresholdDenominator()
    {
        const std::filesystem::path path = createConfigFile(R"({
            "instrument": 42,
            "strategy": {
                "orderQuantity": 100000000,
                "thresholdNumerator": 7,
                "thresholdDenominator": 0
            }
        })");

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        removeConfigFile(path);

        Assert(!result.has_value(), "zero threshold denominator must fail");
        Assert(result.error() == Error::InvalidConfiguration,"invalid error for zero threshold denominator");
    }


    void testLoadFailsForInvalidThresholdRange()
    {
        const std::filesystem::path path = createConfigFile(R"({
            "instrument": 42,
            "strategy": {
                "orderQuantity": 100000000,
                "thresholdNumerator": 11,
                "thresholdDenominator": 10
            }
        })");

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        removeConfigFile(path);

        Assert(!result.has_value(), "invalid threshold range must fail");
        Assert(result.error() == Error::InvalidConfiguration,"invalid error for invalid threshold range");
    }


    void testLoadMultipleExchanges()
    {
        const std::filesystem::path path = createConfigFile(R"({
            "exchanges": [
                {
                    "name": "binance",
                    "marketDataEndpoint": "market-data-1",
                    "executionEndpoint": "execution-1"
                },
                {
                    "name": "test-exchange",
                    "marketDataEndpoint": "market-data-2",
                    "executionEndpoint": "execution-2"
                },
                {
                    "name": "third-exchange",
                    "marketDataEndpoint": "market-data-3",
                    "executionEndpoint": "execution-3"
                }
            ]
        })");

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        removeConfigFile(path);

        Assert(result.has_value(), "multiple exchanges must be loaded");

        const Config& config = *result;

        Assert(config.exchanges.size() == 3, "invalid number of exchanges");
        Assert( config.exchanges[0].name == "binance", "invalid first exchange");
        Assert(config.exchanges[1].name == "test-exchange","invalid second exchange");
        Assert(config.exchanges[2].name == "third-exchange","invalid third exchange");
    }


    void testLoadRecordingConfiguration()
    {
        const std::filesystem::path path = createConfigFile(R"({
            "recording": {
                "enabled": false,
                "directory": "/var/log/trading"
            }
        })");

        const std::expected<Config, Error> result = JsonConfigLoader::load(path);

        removeConfigFile(path);
        Assert(result.has_value(), "recording configuration must be loaded");

        const Config& config = *result;

        Assert(!config.recording.enabled, "recording must be disabled");
        Assert(config.recording.directory == "/var/log/trading","invalid recording directory");
    }

}

void json_config_loader_test()
{
    testLoadValidMinimalConfiguration();
    testLoadFullConfiguration();
    testLoadFailsWhenFileDoesNotExist();
    testLoadFailsForInvalidJson();
    testLoadFailsForInvalidJsonType();
    testLoadFailsForZeroThresholdDenominator();
    testLoadFailsForInvalidThresholdRange();
    testLoadMultipleExchanges();
    testLoadRecordingConfiguration();

    std::cout << "All JsonConfigLoader tests: OK\n";


}
