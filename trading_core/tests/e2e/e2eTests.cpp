/**============================================================================
Name        : e2eTests.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager_test.cpp
============================================================================**/


#include "config.hpp"
#include "timestamp.hpp"
#include "logger.hpp"
#include "json_config_loader.hpp"


namespace e2e_tests
{
    using trading::Price;
    using trading::Quantity;
    using trading::Timestamp;
    using trading::SequenceNumber;

    namespace config = trading::config;

    using config::Config;
    using config::Error;
    using config::ExchangeConfig;
    using config::JsonConfigLoader;


    class TestApplication final
    {
    public:
        explicit TestApplication(const std::filesystem::path& configPath);
        ~TestApplication();

        TestApplication(const TestApplication&) = delete;
        TestApplication& operator=(const TestApplication&) = delete;

        TestApplication(TestApplication&&) = delete;
        TestApplication& operator=(TestApplication&&) = delete;

        void start();
        void stop();

    private:

        Config config;
    };
}


namespace e2e_tests
{
    [[nodiscard]]
    Config loadConfig(const std::filesystem::path& configPath)
    {
        if (const std::expected<Config, Error> result = JsonConfigLoader::load(configPath))
            return *result;
        throw std::runtime_error {"Failed to load configuration: " +configPath.string()};
    }

    [[nodiscard]]
    const ExchangeConfig& findExchange(const Config& config,
                                       const std::string_view name)
    {
        for (const ExchangeConfig& exchange : config.exchanges)
        {
            if (exchange.name == name)
                return exchange;
        }

        throw std::runtime_error {
            "Exchange configuration not found: " + std::string { name }
        };
    }

    TestApplication::TestApplication(const std::filesystem::path& ){
    }

    TestApplication::~TestApplication() {
        stop();
    }

    void TestApplication::start() {

    }

    void TestApplication::stop() {
    }
}

void e2eTests()
{
    e2e_tests::TestApplication test("../../trading_core/tests/e2e/config/test_local.json");
    test.start();
}