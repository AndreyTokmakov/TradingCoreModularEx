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

namespace trading::app {
    class Application;
}

namespace e2e_tests {
    class TestApplication;
}

namespace trading::logging
{
    class LoggerFactory
    {
        class AccessKey
        {
            friend class app::Application;
            friend class e2e_tests::TestApplication;

            AccessKey() = default;
            AccessKey(AccessKey const&) = default;
        };

    public:

        [[nodiscard]]
        static std::shared_ptr<ILogger> createLogger(const LoggingConfiguration& configuration,
                                                     AccessKey accessKey);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_LOGGER_FACTORY_HPP