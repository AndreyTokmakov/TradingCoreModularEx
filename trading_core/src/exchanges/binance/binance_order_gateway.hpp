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

#include <functional>
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
    class BinanceOrderGateway final : public execution::IOrderGateway
    {
    public:
        using SendHandler   = std::function<void(const execution::Order&)>;
        using CancelHandler = std::function<void(OrderId)>;

        explicit BinanceOrderGateway(std::string endpoint) noexcept;

        BinanceOrderGateway(std::string endpoint,
                            SendHandler sendHandler,
                            CancelHandler cancelHandler) noexcept;

        void setSendHandler(SendHandler sendHandler) noexcept;

        void setCancelHandler(CancelHandler cancelHandler) noexcept;

        void send(const execution::Order& order) override;

        void cancel(OrderId orderId) override;

    private:
        std::string endpoint;
        SendHandler sendHandler;
        CancelHandler cancelHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_ORDER_GATEWAY_HPP