/**============================================================================
Name        : test_logger.hpp
Created on  : 14.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_logger.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_TEST_SUPPORT_TEST_LOGGER_HPP
#define TRADINGCOREMODULAREX_TEST_SUPPORT_TEST_LOGGER_HPP

#include "logging/logger.hpp"

#include <algorithm>
#include <print>
#include <string>
#include <string_view>
#include <vector>

namespace trading::testing
{
    class TestLogger final : public logging::ILogger
    {
    public:
        enum class Level
        {
            Trace,
            Debug,
            Info,
            Warning,
            Error,
            Critical
        };

        struct LogRecord
        {
            Level level;
            std::string message;
        };

        explicit TestLogger(const bool printLogs = false) noexcept : printLogs { printLogs }
        {
        }

        [[nodiscard]]
        const std::vector<LogRecord>& records() const noexcept {
            return logRecords;
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return logRecords.empty();
        }

        [[nodiscard]]
        size_t size() const noexcept {
            return logRecords.size();
        }

        [[nodiscard]]
        size_t count(const Level level) const noexcept
        {
            return static_cast<size_t>(std::count_if(logRecords.cbegin(), logRecords.cend(),
            [level](const LogRecord& record) noexcept {
                    return record.level == level;
            }));
        }

        [[nodiscard]]
        bool contains(const std::string_view text) const noexcept
        {
            return std::ranges::any_of(logRecords, [text](const LogRecord& record) noexcept {
                return record.message.find(text) != std::string::npos;
            });
        }

        [[nodiscard]]
        bool contains(const Level level) const noexcept
        {
            return std::ranges::any_of(logRecords,[level](const LogRecord& record) noexcept {
                return record.level == level;
            });
        }

        [[nodiscard]]
        bool contains(const Level level, const std::string_view text) const noexcept
        {
            return std::ranges::any_of(logRecords, [level, text](const LogRecord& record) noexcept {
                return record.level == level && record.message.find(text) != std::string::npos;
            });
        }

        [[nodiscard]]
        bool hasErrors() const noexcept{
            return contains(Level::Error) || contains(Level::Critical);
        }

        [[nodiscard]]
        bool hasNoErrors() const noexcept {
            return !hasErrors();
        }

        void clear() noexcept {
            logRecords.clear();
        }

        void setPrintEnabled(const bool enabled) noexcept {
            printLogs = enabled;
        }

        [[nodiscard]]
        bool isPrintEnabled() const noexcept {
            return printLogs;
        }


    private:

        enum class Color: uint8_t
        {
            Gray,
            Cyan,
            Green,
            Yellow,
            Red,
            Magenta
        };

        [[nodiscard]]
        static constexpr std::string_view levelName(const Level level) noexcept
        {
            switch (level) {
                case Level::Trace:
                    return "TRACE   ";
                case Level::Debug:
                    return "DEBUG   ";
                case Level::Info:
                    return "INFO    ";
                case Level::Warning:
                    return "WARNING ";
                case Level::Error:
                    return "ERROR   ";
                case Level::Critical:
                    return "CRITICAL";
            }
            return "UNKNOWN ";
        }


        [[nodiscard]]
        static constexpr Color levelColor(const Level level) noexcept
        {
            switch (level) {
                case Level::Trace:
                    return Color::Gray;
                case Level::Debug:
                    return Color::Cyan;
                case Level::Info:
                    return Color::Green;
                case Level::Warning:
                    return Color::Yellow;
                case Level::Error:
                    return Color::Red;
                case Level::Critical:
                    return Color::Magenta;
            }
            return Color::Gray;
        }


        [[nodiscard]]
        static constexpr std::string_view colorCode(const Color color) noexcept
        {
            switch (color) {
                case Color::Gray:
                    return "\033[90m";
                case Color::Cyan:
                    return "\033[36m";
                case Color::Green:
                    return "\033[32m";
                case Color::Yellow:
                    return "\033[33m";
                case Color::Red:
                    return "\033[31m";
                case Color::Magenta:
                    return "\033[35m";
            }

            return "\033[0m";
        }

        static constexpr std::string_view ResetColor { "\033[0m" };

        void addRecord(const Level level, std::string message)
        {
            logRecords.emplace_back( level,std::move(message));
            if (printLogs)
                printRecord(logRecords.back());
        }

        static void printRecord(const LogRecord& record)
        {
            const std::string_view name = levelName(record.level);
            std::println("{}[{}]{} {}",
                    colorCode(levelColor(record.level)),
                    name,
                    ResetColor,
                    record.message);
        }

        void traceImpl(std::string msg) override
        {
            addRecord(Level::Trace, std::move(msg));
        }

        void debugImpl(std::string msg) override
        {
            addRecord(Level::Debug, std::move(msg));
        }

        void infoImpl(std::string msg) override
        {
            addRecord(Level::Info, std::move(msg));
        }

        void warnImpl(std::string msg) override
        {
            addRecord(Level::Warning, std::move(msg));
        }

        void errorImpl(std::string msg) override
        {
            addRecord(Level::Error, std::move(msg));
        }

        void criticalImpl(std::string msg) override
        {
            addRecord(Level::Critical, std::move(msg));
        }

        std::vector<LogRecord> logRecords;
        bool printLogs { false };
    };
}

#endif
