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
#include <mutex>

namespace trading::logging
{
    namespace
    {
        std::mutex& getLoggerMutex() noexcept
        {
            static std::mutex mutex;
            return mutex;
        }
    }

    std::shared_ptr<ILogger> LoggerFactory::logger;

    std::shared_ptr<ILogger> LoggerFactory::getLogger() noexcept
    {
        std::lock_guard lock { getLoggerMutex() };
        return logger;
    }

    void LoggerFactory::createLogger(std::shared_ptr<ILogger> newLogger) noexcept
    {
        std::lock_guard lock { getLoggerMutex() };
        logger = std::move(newLogger);
    }

    std::shared_ptr<ILogger>
    LoggerFactory::createLogger(const LoggingConfiguration& configuration)
    {
        auto newLogger = std::make_shared<SpdlogLogger>(configuration);

        {
            std::lock_guard lock { getLoggerMutex() };
            logger = newLogger;
        }

        return newLogger;
    }
}