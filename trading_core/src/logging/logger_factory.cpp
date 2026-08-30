/**============================================================================
Name        : logger_factory.cpp
Created on  : 24.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Factory for creating the trading system logger.
============================================================================**/

#include "logger_factory.hpp"
#include "spdlog_logger.hpp"

namespace trading::logging
{
    std::shared_ptr<ILogger>
    LoggerFactory::createLogger(const LoggingConfiguration& configuration, AccessKey)
    {
        return std::make_shared<SpdlogLogger>(configuration);
    }
}