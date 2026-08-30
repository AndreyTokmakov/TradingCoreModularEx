/**============================================================================
Name        : logger.hpp
Created on  : 24.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Logging abstraction used by the trading system.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_LOGGER_HPP
#define FINANCETECHNOLOGYPROJECTS_LOGGER_HPP

#include <string_view>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>

namespace trading::logging
{
    class ILogger
    {
    public:
        virtual ~ILogger() = default;

        template<typename... Args>
        void trace(fmt::format_string<Args...> format, Args&&... args)
        {
            traceImpl(fmt::format(format, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void debug(fmt::format_string<Args...> format, Args&&... args)
        {
            debugImpl(fmt::format(format, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void info(fmt::format_string<Args...> format, Args&&... args)
        {
            infoImpl(fmt::format(format, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void warn(fmt::format_string<Args...> format, Args&&... args)
        {
            warnImpl(fmt::format(format, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void error(fmt::format_string<Args...> format, Args&&... args)
        {
            errorImpl(fmt::format(format, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void critical(fmt::format_string<Args...> format, Args&&... args)
        {
            criticalImpl(fmt::format(format, std::forward<Args>(args)...));
        }

    private:
        virtual void traceImpl(std::string message) = 0;
        virtual void debugImpl(std::string message) = 0;
        virtual void infoImpl(std::string message) = 0;
        virtual void warnImpl(std::string message) = 0;
        virtual void errorImpl(std::string message) = 0;
        virtual void criticalImpl(std::string message) = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_LOGGER_HPP