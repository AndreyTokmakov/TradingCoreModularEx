/**============================================================================
Name        : order_manager.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager.cpp
============================================================================**/

/*
    OrderManager implementation.

    Order creation flow:

        OrderRequest
             |
             v
        RiskManager
             |
       +-----+-----+
       |           |
    Reject       Accept
       |           |
       v           v
    return 0    create Order
                   |
                   v
                store
                   |
                   v
            gateway.send()

    Execution reports are handled separately and update the locally stored
    order state.

    Position changes are intentionally not performed when an order is created.
    PositionManager must update the position only after an execution report
    confirms an actual trade.
*/

#include "order_manager.hpp"

namespace trading::execution
{
    OrderManager::OrderManager(risk::IRiskManager& riskManager,
                               position::PositionManager& positionManager,
                               IExecutionGateway& gateway) noexcept:
        riskManager { riskManager },
        positionManager { positionManager },
        gateway { gateway }
    {
    }

    OrderCreationResult OrderManager::createOrder(const OrderRequest& request)
    {
        if (request.instrument == InstrumentId { 0 } ||
            request.quantity.isZero() ||
            !request.quantity.isPositive() ||
            request.price.isZero() ||
            !request.price.isPositive())
        {
            return std::unexpected(OrderCreationError::InvalidRequest);
        }

        const position::Position& position = positionManager.getOrDefault(request.instrument);
        const risk::RiskResult riskResult = riskManager.checkOrder(request, position);

        if (riskResult != risk::RiskResult::Accepted)
            return std::unexpected(OrderCreationError::RiskRejected);

        const OrderId orderId = nextOrderId++;

        Order order {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 0 },
            .instrument = request.instrument,
            .side = request.side,
            .type = request.type,
            .price = request.price,
            .quantity = request.quantity,
            .filledQuantity = Quantity {},
            .status = OrderStatus::New
        };

        const auto [it, inserted] = orders.emplace(orderId, order);

        if (!inserted)
            return std::unexpected(OrderCreationError::InvalidRequest);

        gateway.send(it->second);
        return orderId;
    }

    bool OrderManager::applyExecution(const ExecutionReport& report)
    {
        const auto itOrder = orders.find(report.clientOrderId);
        if (itOrder == orders.end())
            return false;

        Order& order = itOrder->second;

        order.exchangeOrderId = report.exchangeOrderId;
        order.status = report.status;
        order.filledQuantity = report.filledQuantity;

        if (report.execType == ExecType::Trade && !positionManager.applyExecution(report))
            return false;

        // Если positionManager.applyExecution(report) вернёт false, OrderManager уже частично изменил Order но наружу вернётся
        // То есть false здесь уже означает не «ничего не обработано», а «обработка была частично выполнена».
        // Это не обязательно ошибка, но семантику bool стоит определить явно.
        // Заменить bool на  enum class ExecutionResult {
        //                            Success,
        //                            UnknownOrder,
        //                            PositionUpdateFailed
        //                    };

        return true;
    }

    const Order* OrderManager::find(const OrderId orderId) const noexcept
    {
        const auto it = orders.find(orderId);
        if (it == orders.end())
            return nullptr;
        return &it->second;
    }

    bool OrderManager::cancel(const OrderId orderId)
    {
        if (const auto it = orders.find(orderId); it == orders.end())
            return false;
        gateway.cancel(orderId);
        return true;
    }
}