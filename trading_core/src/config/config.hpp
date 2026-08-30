/**============================================================================
Name        : config.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Application configuration model independent of external
              configuration formats.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_CONFIG_HPP
#define FINANCETECHNOLOGYPROJECTS_CONFIG_HPP

#include "../core/instrument.hpp"
#include "../core/quantity.hpp"
#include "../risk/risk_limits.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace trading::config
{
    enum class Error: uint8_t
    {
        FileOpenFailed,
        FileReadFailed,
        InvalidFormat,
        InvalidConfiguration
    };


    /**
     * @brief Configuration of the trading strategy.
     *
     * Contains parameters required by the strategy execution pipeline.
     *
     * This structure describes strategy configuration data only. It does not
     * contain or depend on a concrete strategy implementation.
     */
    struct StrategyConfig
    {
        Quantity orderQuantity { 100'000'000 };
        int64_t thresholdNumerator { 7 };
        int64_t thresholdDenominator { 10 };
    };


    /**
     * @brief Configuration of an exchange connection.
     *
     * Describes exchange connectivity parameters required by the application.
     *
     * This structure does not represent a concrete exchange gateway or market
     * data source. Concrete exchange implementations are created by the
     * application composition layer using these parameters.
     */
    struct ExchangeConfig
    {
        std::string name;

        std::string marketDataEndpoint;
        std::string executionEndpoint;
    };


    /**
     * @brief Configuration of application recording.
     *
     * Defines whether recording is enabled and where recorded data should
     * be stored.
     *
     * The configuration does not define the recording implementation or
     * storage format.
     */
    struct RecordingConfig
    {
        bool enabled { true };
        std::filesystem::path directory {};
    };


    /**
     * @brief Canonical application configuration.
     *
     * Config is the representation-independent configuration model used by
     * the application.
     *
     * It intentionally does not depend on JSON, YAML, CSV or any other
     * external configuration format. Configuration loaders are responsible
     * for converting external representations into this model.
     *
     * The application composition layer consumes Config and uses it to
     * construct and configure concrete system components.
     */
    struct Config
    {
        InstrumentId instrument {};

        StrategyConfig strategy {};
        risk::RiskLimits riskLimits {};

        std::vector<ExchangeConfig> exchanges;

        RecordingConfig recording {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_CONFIG_HPP