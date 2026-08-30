/**============================================================================
Name        : spdlog_logger.hpp
Created on  : 24.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Asynchronous spdlog based Logger implementation.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_SPDLOG_LOGGER_HPP
#define FINANCETECHNOLOGYPROJECTS_SPDLOG_LOGGER_HPP

#include "logger.hpp"
#include "logging_configuration.hpp"

#include <memory>

namespace trading::logging
{
    class SpdlogLogger final : public ILogger
    {
    public:
        explicit SpdlogLogger(const LoggingConfiguration& configuration);

        ~SpdlogLogger() override;

        SpdlogLogger(const SpdlogLogger&) = delete;
        SpdlogLogger& operator=(const SpdlogLogger&) = delete;

        SpdlogLogger(SpdlogLogger&&) = delete;
        SpdlogLogger& operator=(SpdlogLogger&&) = delete;

    private:

        void traceImpl(std::string message) override;
        void debugImpl(std::string message) override;
        void infoImpl(std::string message) override;
        void warnImpl(std::string message) override;
        void errorImpl(std::string message) override;
        void criticalImpl(std::string message) override;

    private:
        
        class Impl;
        std::unique_ptr<Impl> impl;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_SPDLOG_LOGGER_HPP