/**============================================================================
Name        : binance_execution_gateway.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance execution gateway implementation.
============================================================================**/

#include "binance_order_gateway.hpp"

#include <utility>

namespace trading::exchanges::binance
{
    BinanceOrderGateway::BinanceOrderGateway(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) }
    {
    }

    BinanceOrderGateway::BinanceOrderGateway(std::string endpoint,
                                             SendHandler sendHandler,
                                             CancelHandler cancelHandler) noexcept :
        endpoint { std::move(endpoint) },
        sendHandler { std::move(sendHandler) },
        cancelHandler { std::move(cancelHandler) }
    {
    }

    void BinanceOrderGateway::setSendHandler(SendHandler handler) noexcept
    {
        sendHandler = std::move(handler);
    }

    void BinanceOrderGateway::setCancelHandler(CancelHandler handler) noexcept
    {
        cancelHandler = std::move(handler);
    }

    void BinanceOrderGateway::send(const execution::Order& order)
    {
        if (!sendHandler)
            return;
        sendHandler(order);
    }

    void BinanceOrderGateway::cancel(const OrderId orderId)
    {
        if (!cancelHandler)
            return;

        cancelHandler(orderId);
    }
}