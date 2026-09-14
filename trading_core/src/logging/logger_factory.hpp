/**============================================================================
Name        : logger_factory.hpp
Created on  : 24.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Factory for creating the trading system logger.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_LOGGER_FACTORY_HPP
#define FINANCETECHNOLOGYPROJECTS_LOGGER_FACTORY_HPP

#include "logger.hpp"
#include "logging_configuration.hpp"

#include <memory>


namespace trading::logging
{
    class LoggerFactory
    {
    public:

        [[nodiscard]]
        static std::shared_ptr<ILogger> getLogger() noexcept;

        static void createLogger(std::shared_ptr<ILogger> logger) noexcept;

        [[nodiscard]]
        static std::shared_ptr<ILogger>
        createLogger(const LoggingConfiguration& configuration);

    private:

        static std::shared_ptr<ILogger> logger;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_LOGGER_FACTORY_HPP