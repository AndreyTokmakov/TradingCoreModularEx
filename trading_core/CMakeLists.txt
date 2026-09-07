cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(trading_core)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 26)

add_compile_options(-c -Wall -Wextra -O3 -std=c++26)

#[[
message(STATUS "[dependency] fetching Boost")
set(BOOST_ENABLE_PYTHON OFF)
set(BOOST_IOSTREAMS_ENABLE_ZLIB ON)
FetchContent_Declare(boost
        GIT_REPOSITORY https://github.com/boostorg/boost.git
        GIT_TAG boost-1.85.0
        GIT_SHALLOW ON
        GIT_PROGRESS ON
        GIT_SUBMODULES ${BOOST_REQD_SUBMODULES}
        CMAKE_ARGS  -DBOOST_IOSTREAMS_ENABLE_ZLIB=ON
        OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(boost)
#]]


# Nlohmann JSON
include_directories(${THIRD_PARTY_DIR}/json/include)
# SPD Log
include_directories(${THIRD_PARTY_DIR}/spdlog/include)



set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})

set(APP         ${PROJECT_DIR}/app)
set(SOURCES     ${PROJECT_DIR}/src)
set(TESTS       ${PROJECT_DIR}/tests)

set(COMMON      ${SOURCES}/common)
set(CONFIG      ${SOURCES}/config)
set(CORE        ${SOURCES}/core)
set(EXECUTION   ${SOURCES}/execution)
set(EXCHANGES   ${SOURCES}/exchanges)
set(LOGGING     ${SOURCES}/logging)
set(MARKET_DATA ${SOURCES}/market_data)
set(METRICS     ${SOURCES}/metrics)
set(POSITION    ${SOURCES}/position)
set(PNL         ${SOURCES}/pnl)
set(RECORDING   ${SOURCES}/recording)
set(RISK        ${SOURCES}/risk)
set(STRATEGY    ${SOURCES}/strategy)

set(BINANCE     ${EXCHANGES}/binance)

include_directories(${CORE})
include_directories(${CONFIG})
include_directories(${COMMON})
include_directories(${MARKET_DATA})
# include_directories(${MARKET_DATA}/model)
# include_directories(${MARKET_DATA}/interfaces)
include_directories(${METRICS})
include_directories(${EXECUTION})
include_directories(${STRATEGY})
include_directories(${POSITION})
include_directories(${RECORDING})
include_directories(${PNL})
include_directories(${RISK})
include_directories(${TESTS})
include_directories(${EXCHANGES})
include_directories(${BINANCE})
include_directories(${LOGGING})

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        ${APP}/application.hpp
        ${APP}/application.cpp

        ${COMMON}/queue.hpp
        ${COMMON}/condition_variable_queue.hpp
        ${COMMON}/ring_buffer.hpp
        ${COMMON}/worker.hpp

        ${CONFIG}/config.hpp
        ${CONFIG}/json_config_loader.hpp
        ${CONFIG}/json_config_loader.cpp
        ${CONFIG}/config_utils.cpp
        ${CONFIG}/config_utils.hpp

        ${CORE}/price.hpp
        ${CORE}/quantity.hpp
        ${CORE}/types.hpp
        ${CORE}/instrument.hpp
        ${CORE}/timestamp.hpp

        ${MARKET_DATA}/model/book_level.hpp
        ${MARKET_DATA}/model/book_update.hpp
        ${MARKET_DATA}/model/market_event.hpp
        ${MARKET_DATA}/model/parse_result.hpp
        ${MARKET_DATA}/model/order_book_levels.hpp
        ${MARKET_DATA}/model/snapshot.hpp

        ${METRICS}/counter.hpp
        ${METRICS}/metrics.hpp
        ${METRICS}/metric_type.hpp
        ${METRICS}/metrics_collector.hpp
        ${METRICS}/metrics_collector.cpp
        # ${METRICS}/hugh_resolution_timer.hpp

        ${MARKET_DATA}/interfaces/market_data_parser.hpp
        ${MARKET_DATA}/interfaces/market_data_source.hpp
        ${MARKET_DATA}/interfaces/snapshot_provider.hpp

        ${MARKET_DATA}/market_data_message_handler.cpp
        ${MARKET_DATA}/market_data_message_handler.hpp
        ${MARKET_DATA}/market_event_dispatcher.cpp
        ${MARKET_DATA}/market_event_dispatcher.hpp
        ${MARKET_DATA}/order_book.hpp
        ${MARKET_DATA}/order_book.cpp
        ${MARKET_DATA}/book_builder_module.hpp
        ${MARKET_DATA}/book_builder_module.cpp
        ${MARKET_DATA}/book_builder.hpp
        ${MARKET_DATA}/book_builder.cpp
        ${MARKET_DATA}/market_data_module.cpp
        ${MARKET_DATA}/market_data_module.hpp

        ${LOGGING}/logger.hpp
        ${LOGGING}/spdlog_logger.hpp
        ${LOGGING}/spdlog_logger.cpp
        ${LOGGING}/logger_factory.hpp
        ${LOGGING}/logger_factory.cpp

        ${EXECUTION}/model/order.hpp
        ${EXECUTION}/model/execution_work_item.hpp
        ${EXECUTION}/model/execution_report.hpp
        ${EXECUTION}/interfaces/execution_gateway.hpp
        ${EXECUTION}/interfaces/execution_report_source.hpp
        ${EXECUTION}/order_manager.hpp
        ${EXECUTION}/order_manager.cpp
        ${EXECUTION}/execution_module.cpp
        ${EXECUTION}/execution_module.hpp
        ${EXECUTION}/execution_report_module.hpp
        ${EXECUTION}/execution_report_module.cpp

        ${EXCHANGES}/exchange_type.hpp
        ${EXCHANGES}/exchange_factory.hpp
        ${EXCHANGES}/exchange_factory_registry.hpp
        ${EXCHANGES}/exchange_factory_registry.cpp


        ${BINANCE}/binance_market_data_source.hpp
        ${BINANCE}/binance_market_data_source.cpp
        ${BINANCE}/binance_market_data_parser.cpp
        ${BINANCE}/binance_market_data_parser.hpp
        ${BINANCE}/binance_execution_gateway.hpp
        ${BINANCE}/binance_execution_gateway.cpp
        ${BINANCE}/binance_execution_report_source.hpp
        ${BINANCE}/binance_execution_report_source.cpp
        ${BINANCE}/binance_snapshot_provider.hpp
        ${BINANCE}/binance_snapshot_provider.cpp
        ${BINANCE}/binance_exchange_factory.hpp
        ${BINANCE}/binance_exchange_factory.cpp


        ${POSITION}/position.hpp
        ${POSITION}/position.cpp
        ${POSITION}/position_manager.hpp
        ${POSITION}/position_manager.cpp

        ${RECORDING}/recorder.hpp
        ${RECORDING}/recording_event.hpp
        ${RECORDING}/trade_recorder.hpp
        ${RECORDING}/trade_recorder.cpp
        ${RECORDING}/recording_module.cpp
        ${RECORDING}/recording_module.hpp

        ${PNL}/pnl.hpp
        ${PNL}/pnl_calculator.hpp
        ${PNL}/pnl_calculator.cpp

        ${RISK}/risk.hpp
        ${RISK}/risk_limits.hpp
        ${RISK}/risk_manager.hpp
        ${RISK}/risk_manager.cpp

        ${STRATEGY}/strategy.hpp
        ${STRATEGY}/signal.hpp
        ${STRATEGY}/imbalance_strategy.hpp
        ${STRATEGY}/imbalance_strategy.cpp
        ${STRATEGY}/strategy_executor.cpp
        ${STRATEGY}/strategy_executor.cpp
        ${STRATEGY}/strategy_module.hpp
        ${STRATEGY}/strategy_module.cpp


        ${TESTS}/core/price_test.cpp
        ${TESTS}/risk/risk_manager_test.cpp
        ${TESTS}/pnl/pnl_calculator_test.cpp
        ${TESTS}/position/position_test.cpp
        ${TESTS}/position/position_manager_test.cpp
        ${TESTS}/recording/trade_recorder_test.cpp
        ${TESTS}/config/json_config_loader_test.cpp
        ${TESTS}/execution/order_manager_test.cpp
        ${TESTS}/market_data/order_book_test.cpp
        ${TESTS}/market_data/book_builder_test.cpp
        ${TESTS}/strategy/imbalance_strategy_test.cpp

        ${TESTS}/modules/book_builder_module_test.cpp
        ${TESTS}/modules/market_data_module_test.cpp

        ${TESTS}/test_support/testing.hpp
        ${TESTS}/test_support/test_execution_gateway.hpp
        ${TESTS}/test_support/test_market_data_source.hpp
        ${TESTS}/test_support/test_market_data_source.cpp
        ${TESTS}/test_support/test_market_data_parser.cpp
        ${TESTS}/test_support/test_market_data_parser.hpp
        ${TESTS}/test_support/test_exchange_factory.hpp

        # ${TESTS}/strategy/strategy_executor_test.cpp
        # ${TESTS}/market_data/market_event_handler_test.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        ${EXTRA_LIBS}
)