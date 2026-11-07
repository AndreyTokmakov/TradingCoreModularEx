/**============================================================================
Name        : binance_execution_report_source.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance execution report source.
============================================================================**/

/*
    BinanceExecutionReportSource represents the inbound execution event
    boundary for Binance.
    The current implementation is intentionally transport-independent.

    Current flow:

        Test / Binance transport
                  |
                  | ExecutionReport
                  v
        BinanceExecutionReportSource
                  |
                  v
        ExecutionReportHandler

    A real Binance WebSocket implementation can later replace the emit()
    call without changing the trading core.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_REPORT_SOURCE_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_REPORT_SOURCE_HPP

#include <string>
#include "execution/interfaces/execution_report_source.hpp"
#include "network/web_socket_client.hpp"

namespace trading::exchanges::binance
{
    class BinanceExecutionReportSource final: public execution::IExecutionReportSource
    {
    public:
        explicit BinanceExecutionReportSource(std::string endpoint) noexcept;
        BinanceExecutionReportSource(std::string endpoint, std::string api_key, std::string secret_key) noexcept;

        BinanceExecutionReportSource(const BinanceExecutionReportSource&) = delete;
        BinanceExecutionReportSource& operator=(const BinanceExecutionReportSource&) = delete;

        BinanceExecutionReportSource(BinanceExecutionReportSource&&) = delete;
        BinanceExecutionReportSource& operator=(BinanceExecutionReportSource&&) = delete;

        void start() override;
        void stop() override;

        void setReportHandler(execution::IExecutionReportHandler& handler) override;

    private:
        [[nodiscard]]
        std::optional<std::string> createSubscribeRequest() const;

    private:
        execution::IExecutionReportHandler* reportHandler { nullptr };

        std::string apiKey;
        std::string secretKey;

        boost::asio::io_context ioContext;
        boost::asio::ssl::context sslContext;
        network::WebSocketClient wsClient;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_REPORT_SOURCE_HPP