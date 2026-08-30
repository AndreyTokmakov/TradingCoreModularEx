/**============================================================================
Name        : spdlog_logger.cpp
Created on  : 24.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Asynchronous spdlog based Logger implementation.
============================================================================**/

#include "spdlog_logger.hpp"

#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <utility>

namespace
{
    constexpr std::size_t AsyncQueueSize { 8'192 };
    constexpr std::size_t AsyncThreadCount { 1 };

    void initializeThreadPool()
    {
        static std::once_flag initializationFlag;

        std::call_once(initializationFlag, [] {spdlog::init_thread_pool(
            AsyncQueueSize,
            AsyncThreadCount);
        });
    }
}

namespace trading::logging
{
    class SpdlogLogger::Impl
    {
    public:
        explicit Impl(const LoggingConfiguration& configuration): loggerName { configuration.loggerName }
        {
            initializeThreadPool();

            const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            const auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    configuration.logFilePath,
                    configuration.maxFileSize,
                    configuration.maxFiles
            );

            std::vector<spdlog::sink_ptr> sinks { consoleSink, fileSink };

            logger = std::make_shared<spdlog::async_logger>(
                loggerName,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::overrun_oldest
            );

            logger->set_level(spdlog::level::trace);
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        }

        ~Impl() noexcept = default;

        std::string loggerName;
        std::shared_ptr<spdlog::async_logger> logger;
    };

    SpdlogLogger::SpdlogLogger(const LoggingConfiguration& configuration):
        impl { std::make_unique<Impl>(configuration) }
    {
    }

    SpdlogLogger::~SpdlogLogger() = default;

    void SpdlogLogger::traceImpl(const std::string message)
    {
        impl->logger->trace(message);
    }

    void SpdlogLogger::debugImpl(const std::string message)
    {
        impl->logger->debug(message);
    }

    void SpdlogLogger::infoImpl(const std::string message)
    {
        impl->logger->info(message);
    }

    void SpdlogLogger::warnImpl(const std::string message)
    {
        impl->logger->warn(message);
    }

    void SpdlogLogger::errorImpl(const std::string message)
    {
        impl->logger->error(message);
    }

    void SpdlogLogger::criticalImpl(const std::string message)
    {
        impl->logger->critical(message);
    }
}