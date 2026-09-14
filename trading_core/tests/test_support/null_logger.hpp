/**============================================================================
Name        : null_logger.hpp
Created on  : 14.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : null_logger.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_TEST_SUPPORT_NULL_LOGGER_HPP
#define TRADINGCOREMODULAREX_TEST_SUPPORT_NULL_LOGGER_HPP

#include "logger.hpp"
#include <print>

namespace trading::testing
{
    class NullLogger final : public logging::ILogger
    {
    public:
        explicit NullLogger(const bool printLogs = false) : print(printLogs) {}

    private:

        void traceImpl(std::string msg) override {
            if (print) {
                std::println("[TRACE   ] {}", msg);
            }
        }

        void debugImpl(std::string msg) override {
            if (print) {
                std::println("[DEBUG   ] {}", msg);
            }
        }

        void infoImpl(std::string msg) override {
            if (print) {
                std::println("[INFO    ] {}", msg);
            }
        }

        void warnImpl(std::string msg) override {
            if (print) {
                std::println("[WARNING ] {}", msg);
            }
        }

        void errorImpl(std::string msg) override {
            if (print) {
                std::println("[ERROR   ] {}", msg);
            }
        }

        void criticalImpl(std::string msg) override {
            if (print) {
                std::println("[CRITICAL] {}", msg);
            }
        }

        bool print { false };
    };
}

#endif
