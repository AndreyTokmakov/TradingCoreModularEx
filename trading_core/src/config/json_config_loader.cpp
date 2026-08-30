/**============================================================================
Name        : json_config_loader.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : JSON configuration loader implementation.
============================================================================**/

#include "json_config_loader.hpp"

#include <nlohmann/json.hpp>

#include <fstream>

namespace trading::config
{
    namespace
    {
        [[nodiscard]]
        bool isValidStrategyConfig(const StrategyConfig& strategy) noexcept
        {
            return strategy.thresholdDenominator > 0 &&
                   strategy.thresholdNumerator >= 0 &&
                   strategy.thresholdNumerator <= strategy.thresholdDenominator;
        }
    }


    std::expected<Config, Error> JsonConfigLoader::load(const std::filesystem::path& configPath)
    {
        std::ifstream file { configPath };
        if (!file.is_open())
            return std::unexpected(Error::FileOpenFailed);

        try
        {
            const nlohmann::json json = nlohmann::json::parse(file);

            if (!file.good() && !file.eof())
                return std::unexpected(Error::FileReadFailed);

            Config config {};

            if (json.contains("instrument"))
            {
                config.instrument = InstrumentId {
                    json.at("instrument").get<InstrumentId>()
                };
            }

            if (json.contains("strategy"))
            {
                const auto& strategy = json.at("strategy");

                if (strategy.contains("orderQuantity")) {
                    config.strategy.orderQuantity = Quantity { strategy.at("orderQuantity").get<Quantity::Value>()};
                }

                if (strategy.contains("thresholdNumerator")) {
                    config.strategy.thresholdNumerator = strategy.at("thresholdNumerator").get<int64_t>();
                }

                if (strategy.contains("thresholdDenominator")){
                    config.strategy.thresholdDenominator = strategy.at("thresholdDenominator").get<int64_t>();
                }
            }

            if (!isValidStrategyConfig(config.strategy))
                return std::unexpected(Error::InvalidConfiguration);

            if (json.contains("exchanges"))
            {
                for (const auto& exchangeJson : json.at("exchanges"))
                {
                    ExchangeConfig exchange {};

                    if (exchangeJson.contains("name"))
                        exchange.name = exchangeJson.at("name").get<std::string>();

                    if (exchangeJson.contains("marketDataEndpoint"))
                    {
                        exchange.marketDataEndpoint = exchangeJson.at("marketDataEndpoint").get<std::string>();
                    }

                    if (exchangeJson.contains("executionEndpoint"))
                    {
                        exchange.executionEndpoint = exchangeJson.at("executionEndpoint").get<std::string>();
                    }

                    config.exchanges.push_back(std::move(exchange));
                }
            }

            if (json.contains("recording"))
            {
                const auto& recording = json.at("recording");

                if (recording.contains("enabled"))
                    config.recording.enabled = recording.at("enabled").get<bool>();

                if (recording.contains("directory"))
                {
                    config.recording.directory = recording.at("directory").get<std::string>();
                }
            }

            if (json.contains("risk"))
            {
                const auto& risk = json.at("risk");

                if (risk.contains("maxOrderQuantity"))
                {
                    config.riskLimits.maxOrderQuantity = Quantity { risk.at("maxOrderQuantity").get<Quantity::Value>()};
                }

                if (risk.contains("maxPositionQuantity"))
                {
                    config.riskLimits.maxPositionQuantity = Quantity { risk.at("maxPositionQuantity").get<Quantity::Value>() };
                }

                if (risk.contains("maxNotional"))
                {
                    config.riskLimits.maxNotional = Price { risk.at("maxNotional").get<Price::Value>()};
                }
            }

            return config;
        }
        catch (const nlohmann::json::parse_error&)
        {
            return std::unexpected(Error::InvalidFormat);
        }
        catch (const nlohmann::json::type_error&)
        {
            return std::unexpected(Error::InvalidConfiguration);
        }
        catch (const nlohmann::json::out_of_range&)
        {
            return std::unexpected(Error::InvalidConfiguration);
        }
    }
}