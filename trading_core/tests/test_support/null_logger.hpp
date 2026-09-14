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
    private:

        void traceImpl(std::string) override
        {
        }

        void debugImpl(std::string) override
        {
        }

        void infoImpl(std::string msg) override
        {
            std::println("{}", msg);
        }

        void warnImpl(std::string) override
        {
        }

        void errorImpl(std::string) override
        {
        }

        void criticalImpl(std::string) override
        {
        }
    };
}

#endif
