/**============================================================================
Name        : binance_order_gateway.hpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance implementation of the execution gateway.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_ORDER_GATEWAY_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_ORDER_GATEWAY_HPP

#include "order_management/interfaces/order_gateway.hpp"
#include "network/rest_client.hpp"

#include <string>

/**
 * Binance implementation of IOrderGateway.
 *
 * BinanceOrderGateway is responsible for translating internal Order
 * and cancel requests into Binance-specific execution requests.
 *
 * The gateway belongs to the exchange integration layer. Higher-level
 * execution components such as OrderManager must depend only on the
 * IOrderGateway interface and must not contain Binance-specific logic.
 *
 * Execution flow:
 *
 *     Strategy
 *         |
 *         v
 *     StrategyExecutor
 *         |
 *         v
 *     OrderManager
 *         |
 *         v
 *     IOrderGateway
 *         |
 *         v
 *     BinanceOrderGateway
 *         |
 *         v
 *     Binance execution API
 *
 * The actual network transport is intentionally injected through
 * callbacks. This keeps the gateway independent from a particular HTTP
 * or WebSocket implementation and makes the component easy to test.
 *
 * Execution reports travel in the opposite direction:
 *
 *     Binance execution API
 *         |
 *         v
 *     ExecutionReport
 *         |
 *         v
 *     ExecutionReportHandler
 *         |
 *         v
 *     OrderManager / PositionManager / Recorder
 */

namespace trading::exchanges::binance
{
    using order_management::OrderSubmissionResult;
    using order_management::OrderCancellationResult;

    enum class BinanceApiErrorCode : std::uint8_t
    {
        InvalidRequest,
        SigningFailed,
        TransportError,
        ExchangeRejected,
        InvalidResponse
    };

    struct BinanceApiError
    {
        BinanceApiErrorCode code { BinanceApiErrorCode::InvalidResponse };
        std::int64_t exchangeErrorCode { 0 };
        std::string message;
    };

#if 0
    struct ApiCredentials
    {
        std::string apiKey;
        std::string secretKey;
    };
#endif

    class BinanceOrderGateway final : public order_management::IOrderGateway
    {
    public:

        BinanceOrderGateway(std::string endpoint,
                            std::string api_key,
                            std::string secret_keys,
                            std::string symbol = "BTCUSDT",
                            std::uint64_t recvWindow = 5000);

        BinanceOrderGateway(const BinanceOrderGateway&) = delete;
        BinanceOrderGateway& operator=(const BinanceOrderGateway&) = delete;

        using OrderResponse = std::expected<OrderSubmissionResult, BinanceApiError>;
        using CancelResponse = std::expected<OrderCancellationResult, BinanceApiError>;

        void send(const order_management::Order& order) override;
        void cancel(ExchangeOrderId exchangeOrderId) override;

    private:
        using RestResponse = network::RestClient::Response;
        using ApiResponse = std::expected<network::HttpResponse, BinanceApiError>;

        using Handler = void (BinanceOrderGateway::*)(ApiResponse);

        void asyncSignedRequest(boost::beast::http::verb method,
                                std::string query,
                                Handler handler);
    private:

        std::string endpoint;

        boost::asio::io_context ioContext;
        boost::asio::ssl::context sslContext;
        network::RestClient restClient;

        // TODO:  --> ApiCredentials
        std::string apiKey;
        std::string secretKey;

        std::string symbol;
        std::uint64_t recvWindow;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_ORDER_GATEWAY_HPP