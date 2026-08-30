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

#include "execution_gateway.hpp"

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
        const execution::Order& lastOrder() const
        {
            return sentOrders.back();
        }

        [[nodiscard]]
        const std::vector<execution::Order>& orders() const noexcept
        {
            return sentOrders;
        }

        [[nodiscard]]
        std::size_t orderCount() const noexcept
        {
            return sentOrders.size();
        }

        [[nodiscard]]
        const std::vector<OrderId>& cancelledOrderIds() const noexcept
        {
            return cancelledOrders;
        }

        void clear() noexcept
        {
            sentOrders.clear();
            cancelledOrders.clear();
        }

    private:
        std::vector<execution::Order> sentOrders;
        std::vector<OrderId> cancelledOrders;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_GATEWAY_HPP
