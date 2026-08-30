/**============================================================================
Name        : logging_configuration.hpp
Created on  : 24.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Configuration of the trading system logging infrastructure.

The configuration belongs to the application/infrastructure layer and is
passed to the logging factory when the application creates its Logger.

SpdlogLogger does not define application-specific logging configuration.
In particular, file names, rotation limits and logger names are not hardcoded
inside the logger implementation.

The asynchronous spdlog thread pool is process-global and is initialized by
SpdlogLogger independently from this configuration.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_LOGGING_CONFIGURATION_HPP
#define FINANCETECHNOLOGYPROJECTS_LOGGING_CONFIGURATION_HPP

#include <cstddef>
#include <string>

namespace trading::logging
{
    struct LoggingConfiguration
    {
        std::string loggerName { "trading" };
        std::string logFilePath { "trading.log" };

        std::size_t maxFileSize { 10 * 1024 * 1024 };
        std::size_t maxFiles { 5 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_LOGGING_CONFIGURATION_HPP