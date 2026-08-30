/**============================================================================
Name        : json_config_loader.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : JSON configuration loader interface.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_JSON_CONFIG_LOADER_HPP
#define FINANCETECHNOLOGYPROJECTS_JSON_CONFIG_LOADER_HPP

#include "config.hpp"

#include <expected>
#include <filesystem>

namespace trading::config
{
    /**
     * @brief Loads application configuration from a JSON file.
     *
     * JsonConfigLoader is responsible only for translating JSON data into
     * the canonical Config representation.
     *
     * It does not construct application components and does not know about
     * concrete exchange gateways, strategies, recorders or other runtime
     * objects.
     */
    class JsonConfigLoader final
    {
    public:
        [[nodiscard]]
        static std::expected<Config, Error> load(const std::filesystem::path& configPath);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_JSON_CONFIG_LOADER_HPP