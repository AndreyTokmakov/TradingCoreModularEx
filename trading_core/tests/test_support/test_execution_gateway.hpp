/**============================================================================
Name        : test_execution_gateway.сpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : mock_execution_gateway.сpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_GATEWAY_HPP
#define FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_GATEWAY_HPP

#include "interfaces/execution_gateway.hpp"

#include <vector>

namespace trading::testing
{
    class TestExecutionGateway final : public execution::IExecutionGateway
    {
    public:

        void send(const execution::Order& order) override
        {
            sentOrders.push_back(order);
        }

        void cancel(const OrderId orderId) override
        {
            cancelledOrders.push_back(orderId);
        }

        [[nodiscard]]
        bool hasOrder() const noexcept
        {
            return !sentOrders.empty();
        }

        [[nodiscard]]
        const execution::Order& lastSendOrder() const noexcept
        {
            return sentOrders.back();
        }

        [[nodiscard]]
        OrderId getLastCancelledOrderId() const noexcept
        {
            return cancelledOrders.back();
        }

        [[nodiscard]]
        size_t sendOrdersCount() const noexcept
        {
            return sentOrders.size();
        }

        [[nodiscard]]
        size_t cancelCountCount() const noexcept
        {
            return cancelledOrders.size();
        }

        execution::Order& getOrderByIndex(const size_t index) {
            return sentOrders[index];
        }

    private:

        std::vector<execution::Order> sentOrders;
        std::vector<OrderId> cancelledOrders;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_GATEWAY_HPP
