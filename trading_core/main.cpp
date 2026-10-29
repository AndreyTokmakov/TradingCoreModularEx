/**============================================================================
Name        : main.cpp
Created on  : 
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : 
============================================================================**/

#include <iostream>
#include <filesystem>
#include <vector>
#include <string_view>

#include "app/application.hpp"
#include "exchanges/exchange_factory_registry.hpp"
#include "exchanges/binance/binance_market_data_source.hpp"
#include "exchanges/binance/binance_market_data_parser.hpp"
#include "exchanges/binance/binance_snapshot_provider.hpp"
#include "logging/logger_factory.hpp"
#include "common/error_code.hpp"
#include "config/config_utils.hpp"
#include "test_support/test_logger.hpp"
#include "test_support/debug_helpers.hpp"

#include "network/web_socket_client.hpp"
#include "network/rest_client.hpp"

void price_test();
void order_book_test();
void order_manager_test();
void book_builder_test();

void market_data_module_test();
void book_builder_module_restore_book_test();
void book_builder_module_test();
void marketdata_bookbuilder_strategy_integrataion();
void strategy_module_test();
void execution_module_test();
void trade_module_test();
void exchange_to_gateway_integration_test();

void market_event_handler_test();
void pnl_calculator_test();
void risk_manager_test();
void trade_recorder_test();
void position_test();
void position_manager_test();
void imbalance_strategy_test();
void strategy_executor_test();
void json_config_loader_test();
void trade_processor_test();

void trading_integration_test();
void trading_inbound_integration_test();



// TODO: - Improvements
//  - Заменить std::map<K,V> для PriceLevel-ов ---> DenseMap

// TODO: Metics
//  - Put Collection on CPU-5
//  - Grafana / Prometheus ?

// TODO:
//  ---- > where to use PnLCalculator ??

/**  Сейчас есть в PositionManager есть applyExecution и applyTrade
 *   разобраться какое нужен и какой оставить
 *   попроавить тесты

bool PositionManager::applyExecution(const execution::ExecutionReport& report)
    {
        if (report.execType != ExecType::Trade || report.quantity.isZero())
            return false;

        auto [it, inserted] = positions.try_emplace(report.instrument, report.instrument);

        Position& position = it->second;
        position.applyTrade(report.side, report.price, report.quantity);

        return true;
    }

    bool PositionManager::applyTrade(const InstrumentId instrument,
                                     const Side side,
                                     const Price price,
                                     const Quantity quantity)
    {
        if (quantity.isZero())
            return false;

        auto [it, inserted] = positions.try_emplace(instrument, instrument);

        Position& position = it->second;
        position.applyTrade(side, price, quantity);

        return true;
    }
**/

/**
| Module              | Metric                          | Когда increment                    |
| ------------------- | ------------------------------- | ---------------------------------- |
| `MarketDataSource`  | `MarketDataReceived`            | raw message успешно получен        |
| `MarketDataSource`  | `MarketDataReceiveErrors`       | ошибка получения                   |
| `MarketDataParser`  | `MarketDataParsed`              | message успешно parsed             |
| `MarketDataParser`  | `MarketDataParseErrors`         | parse failed                       |
| `BookBuilderModule` | `MarketDataSnapshots`           | snapshot получен                   |
| `BookBuilderModule` | `MarketDataSnapshotApplyFailed` | snapshot не применён               |
| `BookBuilderModule` | `MarketDataUpdates`             | `BookUpdate` получен для обработки |
| `OrderBook`         | `OrderBookUpdates`              | update реально применён            |
| `OrderBook`         | `OrderBookSequenceErrors`       | обнаружен sequence gap             |
| `StrategyModule`    | `StrategySignals`               | signal создан                      |
| `StrategyModule`    | `StrategySignalsRejected`       | strategy отклонила signal          |
| `RiskManager`       | `RiskChecks`                    | выполнен risk check                |
| `RiskManager`       | `RiskAccepted`                  | risk check passed                  |
| `RiskManager`       | `RiskRejected`                  | risk check failed                  |
| `ExecutionModule`   | `ExecutionRequests`             | request передан на execution       |
| `ExecutionModule`   | `ExecutionReports`              | report обработан                   |
| `ExecutionModule`   | `ExecutionAcknowledgements`     | ACK получен                        |
| `ExecutionModule`   | `ExecutionRejects`              | execution reject                   |
| `ExecutionModule`   | `ExecutionFills`                | fill                               |
| `ExecutionModule`   | `ExecutionPartiallyFilled`      | partial fill                       |
| `ExecutionModule`   | `ExecutionFullyFilled`          | full fill                          |
| `ExecutionModule`   | `ExecutionCancels`              | cancel report                      |
| `Gateway`           | `GatewayConnections`            | connection established             |
| `Gateway`           | `GatewayDisconnections`         | connection lost                    |
| `Gateway`           | `GatewayReconnects`             | reconnect completed                |
| `Gateway`           | `GatewayErrors`                 | gateway error                      |
| `Gateway`           | `GatewayMessagesSent`           | message sent                       |
| `Gateway`           | `GatewayMessagesReceived`       | message received                   |

------------  Добавить метрики -----------------------

    QueueDepth
    ActiveOrders
    OpenPositions
    CurrentExposure
    ConnectedGateways

    order_queue_depth = 37
    active_orders = 12

------------ Расширить тип метрик:

    metrics.increment<MetricType::MarketDataReceived>();
    metrics.set<GaugeType::QueueDepth>(queue.size());
    metrics.observe<MetricType::StrategyLatency>(latency);

----------- ORDER BOOK ----------------

    using Levels    = std::map<Price, Quantity>;
    using Levels    = std::vector<std::pair<Price, Quantity>>;
**/

namespace
{
    using LoggerFactory = trading::logging::LoggerFactory;
    using LoggingConfiguration = trading::logging::LoggingConfiguration;


    [[maybe_unused]]
    void runApp(const std::vector<std::string_view>& parameters)
    {
        const std::filesystem::path configPath = parameters.empty() ?  "config.json" : parameters.front();
        try
        {
            const auto _  = LoggerFactory::createLogger(LoggingConfiguration{});

            trading::app::Application application { configPath };
            application.start();

            // Application lifetime will be controlled here.
            // TODO: Add signal handling / graceful shutdown.
        }
        catch (const std::exception& error)
        {
            std::cerr << "Application startup failed: "<< error.what() << '\n';
        }
    }

    [[maybe_unused]]
    void runTests(const std::vector<std::string_view>& )
    {
        const auto nullLogger = std::make_shared<trading::testing::TestLogger>(true);
        LoggerFactory::createLogger(nullLogger);

        // market_event_handler_test();
        // strategy_executor_test();

        price_test();
        json_config_loader_test();
        trade_recorder_test();
        pnl_calculator_test();
        risk_manager_test();
        position_test();
        position_manager_test();
        order_book_test();
        order_manager_test();
        book_builder_test();
        imbalance_strategy_test();
        trade_processor_test();

        market_data_module_test();
        book_builder_module_test();
        strategy_module_test();
        execution_module_test();
        trade_module_test();
    }

    [[maybe_unused]]
    void runSingleTest(const std::vector<std::string_view>& )
    {
        const auto nullLogger = std::make_shared<trading::testing::TestLogger>(true);
        LoggerFactory::createLogger(nullLogger);

        market_data_module_test();
    }
}

namespace binance_tests
{
    namespace market_data = trading::market_data;
    namespace binance = trading::exchanges::binance;
    namespace network = trading::network;
    namespace common = trading::common;

    using namespace trading::config;
    using common::ErrorCode;
    using common::CVQueue;
    using trading::recording::RecordingEvent;
    using market_data::Snapshot;
    using market_data::ParseResult;
    using trading::exchanges::IExchangeFactory;
    using trading::exchanges::ExchangeFactoryRegistry;
    using trading::exchanges::ExchangeType;

    namespace asio = boost::asio;
    namespace ssl = asio::ssl;

    using namespace std::string_view_literals;

    // TODO:
    //  - WebSocket Client
    //  - REST Client
    //  - IBinanceParser  (using SIMD Json)
    //  - IBinanceSnapshotProvider
    //  - IBinanceMarketDataSource
    //  - Test With SourceProvider (CPU-1) + BookBuilder (CPU-2)
    //  -
    //  - Support Using Configuration !

    constexpr std::string binanceApiHost { "api.binance.com" };

    void testBinanceWsClient()
    {
        const std::unique_ptr<market_data::IMarketDataSource> marketDataSource
            = std::make_unique<binance::BinanceMarketDataSource>(binanceApiHost);

        marketDataSource->start();
    }

    void testBinanceClient()
    {
        const std::unique_ptr<market_data::ISnapshotProvider> snapshotProvider
            = std::make_unique<binance::BinanceSnapshotProvider>(binanceApiHost);

        const std::expected<Snapshot, ErrorCode> snapshot = snapshotProvider->getSnapshot("BTCUSDT", 40);
        std::cout << snapshot.value() << std::endl;
    }

    void marketDataModuleTest(const std::vector<std::string_view>& parameters)
    {
        const auto nullLogger = std::make_shared<trading::testing::TestLogger>(true);
        LoggerFactory::createLogger(nullLogger);

        const std::filesystem::path configPath = parameters.empty() ?  "config.json" : parameters.front();
        const Config config { loadConfig(configPath) };
        const std::unique_ptr<IExchangeFactory> exchangeFactory {
            ExchangeFactoryRegistry::createFactory(ExchangeType::Binance)
        };

        CVQueue<market_data::BookUpdate> bookUpdateQueue;
        CVQueue<market_data::Trade> tradeQueue;

        market_data::MarketDataModule marketDataModule {
            config, bookUpdateQueue, tradeQueue, *exchangeFactory
        };

        marketDataModule.start();
    }

    void bookBuilderModuleTests(const std::vector<std::string_view>& parameters)
    {
        const auto nullLogger = std::make_shared<trading::testing::TestLogger>(true);
        LoggerFactory::createLogger(nullLogger);

        const std::filesystem::path configPath = parameters.empty() ?  "config.json" : parameters.front();
        CVQueue<market_data::BookUpdate> bookUpdateQueue;
        CVQueue<market_data::MarketEvent> strategyEventQueue;
        CVQueue<RecordingEvent> recordingEventQueue;

        const Config config { loadConfig(configPath) };
        const std::unique_ptr<IExchangeFactory> exchangeFactory {
            ExchangeFactoryRegistry::createFactory(ExchangeType::Binance)
        };

        trading::order_book::BookBuilderModule bookBuilderModule {
            config, bookUpdateQueue, strategyEventQueue, recordingEventQueue, *exchangeFactory
        };

        bookBuilderModule.run();
    }

    void BookBuilder_and_MarketData_Modules(const std::vector<std::string_view>& parameters)
    {
        const auto nullLogger = std::make_shared<trading::testing::TestLogger>(true);
        LoggerFactory::createLogger(nullLogger);

        const std::filesystem::path configPath = parameters.empty() ?  "config.json" : parameters.front();

        CVQueue<market_data::BookUpdate> bookUpdateQueue;
        CVQueue<market_data::MarketEvent> strategyEventQueue;
        CVQueue<RecordingEvent> recordingEventQueue;
        CVQueue<market_data::Trade> tradeQueue;

        const Config config { loadConfig(configPath) };
        const std::unique_ptr<IExchangeFactory> exchangeFactory {
            ExchangeFactoryRegistry::createFactory(ExchangeType::Binance)
        };

        market_data::MarketDataModule marketDataModule {
            config, bookUpdateQueue, tradeQueue, *exchangeFactory
        };

        trading::order_book::BookBuilderModule bookBuilderModule {
            config, bookUpdateQueue, strategyEventQueue, recordingEventQueue, *exchangeFactory
        };

        std::jthread marketDataThread {[&] {
            marketDataModule.start(); // FIXME <---- No block the thread
        }};

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        bookBuilderModule.start();
    }

    void test(const std::vector<std::string_view>& parameters)
    {
        // wsClient();
        // testBinanceWsClient();
        // marketDataModuleTest(parameters);

        // testRestClient();
        // testBinanceClient();
        // bookBuilderModuleTests(parameters);

        BookBuilder_and_MarketData_Modules(parameters);
    }
}

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> parameters(argv + 1, argv + argc);

    // runApp(parameters);
    // runTests(parameters);
    // runSingleTest(parameters);
    binance_tests::test(parameters);

    return EXIT_SUCCESS;
}
