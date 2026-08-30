/**============================================================================
Name        : order_manager_test.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager_test.cpp
============================================================================**/

#include "order_manager.hpp"
#include "test_support/testing.hpp"

#include <iostream>

using trading::ExchangeOrderId;
using trading::InstrumentId;
using trading::OrderId;
using trading::OrderStatus;
using trading::OrderType;
using trading::Price;
using trading::Quantity;
using trading::Side;
using trading::ExecType;

using trading::execution::ExecutionReport;
using trading::execution::IExecutionGateway;
using trading::execution::Order;
using trading::execution::OrderManager;
using trading::execution::OrderRequest;
using trading::risk::IRiskManager;
using trading::risk::RiskResult;
using trading::risk::RiskReason;
using trading::position::Position;
using trading::position::PositionManager;

namespace
{
    using testing::Assert;
    using trading::execution::OrderCreationResult;

    class TestRiskManager final : public IRiskManager
    {
    public:
        RiskResult checkOrder(const OrderRequest&,
                                             const Position&) override
        {
            checkCount++;
            return result;
        }

        [[nodiscard]]
        RiskReason lastReason() const noexcept override
        {
            return reason;
        }

        RiskResult result { RiskResult::Accepted };
        RiskReason reason { RiskReason::None };
        uint32_t checkCount { 0 };
        Position::Value checkedPositionQuantity { 0 };
    };

    class TestExecutionGateway final : public IExecutionGateway
    {
    public:
        void send(const Order& order) override
        {
            sentOrder = order;
            sendCount++;
        }

        void cancel(const OrderId orderId) override
        {
            cancelledOrderId = orderId;
            cancelCount++;
        }

        [[nodiscard]]
        const Order& sentOrderValue() const noexcept
        {
            return sentOrder;
        }

        [[nodiscard]]
        OrderId cancelledOrderIdValue() const noexcept
        {
            return cancelledOrderId;
        }

        [[nodiscard]]
        uint32_t sendCountValue() const noexcept
        {
            return sendCount;
        }

        [[nodiscard]]
        uint32_t cancelCountValue() const noexcept
        {
            return cancelCount;
        }

    private:
        Order sentOrder;
        OrderId cancelledOrderId { 0 };
        uint32_t sendCount { 0 };
        uint32_t cancelCount { 0 };
    };

    void testCreateOrder()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderCreationResult result = manager.createOrder(request);
        Assert(result.has_value(), "order creation must succeed");
        Assert(riskManager.checkCount == 1, "risk manager must be called once");
        //Assert(riskManager.checkedPositionQuantity == 0, "new position must be flat");

        const OrderId orderId = *result;
        Assert(orderId == 1, "first order id must be one");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "created order must exist");
        Assert(order->clientOrderId == orderId, "invalid client order id");
        Assert(order->instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(order->side == Side::Buy, "invalid side");
        Assert(order->type == OrderType::Limit, "invalid order type");
        Assert(order->price == Price { 6'500'000'000'000 }, "invalid price");
        Assert(order->quantity == Quantity { 100'000'000 }, "invalid quantity");
        Assert(order->filledQuantity.isZero(), "new order must have zero filled quantity");
        Assert(order->status == OrderStatus::New, "new order must have New status");
    }

    void testOrderIdsAreUnique()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderCreationResult first = manager.createOrder(request);
        const OrderCreationResult second = manager.createOrder(request);
        const OrderCreationResult third = manager.createOrder(request);

        Assert(first.has_value(), "first order creation must succeed");
        Assert(second.has_value(), "second order creation must succeed");
        Assert(third.has_value(), "third order creation must succeed");

        Assert(first.value() == 1, "first order id must be one");
        Assert(second.value() == 2, "second order id must be two");
        Assert(third .value()== 3, "third order id must be three");

        Assert(manager.find(first.value()) != nullptr, "first order must exist");
        Assert(manager.find(second.value()) != nullptr, "second order must exist");
        Assert(manager.find(third.value()) != nullptr, "third order must exist");
    }


    void testOrderIsSentToGateway()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 42 },
            .side = Side::Sell,
            .type = OrderType::Limit,
            .price = Price { 6'500'001'000'000 },
            .quantity = Quantity { 200'000'000 }
        };

        const OrderCreationResult result = manager.createOrder(request);
        Assert(result.has_value(), "order creation must succeed");
        Assert(gateway.sendCountValue() == 1, "gateway send must be called once");

        const OrderId orderId = result.value();
        const Order& sentOrder = gateway.sentOrderValue();

        Assert(sentOrder.clientOrderId == orderId, "invalid sent order id");
        Assert(sentOrder.instrument == InstrumentId { 42 }, "invalid sent instrument");
        Assert(sentOrder.side == Side::Sell, "invalid sent side");
        Assert(sentOrder.type == OrderType::Limit, "invalid sent order type");
        Assert(sentOrder.price == Price { 6'500'001'000'000 }, "invalid sent price");
        Assert(sentOrder.quantity == Quantity { 200'000'000 }, "invalid sent quantity");
    }

    void testRiskManagerRejectsOrder()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        riskManager.result = RiskResult::Rejected;

        OrderManager manager { gateway, riskManager, positionManager };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderCreationResult result = manager.createOrder(request);

        Assert(!result.has_value(), "rejected order must not be created");
        Assert(
            result.error() == trading::execution::OrderCreationError::RiskRejected,
            "invalid order creation error"
        );
        Assert(riskManager.checkCount == 1, "risk manager must be called once");
        Assert(gateway.sendCountValue() == 0, "rejected order must not be sent");
    }

    void testFindUnknownOrder()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        const OrderManager manager { gateway, riskManager, positionManager };

        const Order* order = manager.find(OrderId { 42 });

        Assert(order == nullptr, "unknown order must not be found");
    }

    void testApplyNewExecutionReport()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderCreationResult result = manager.createOrder(request);
        Assert(result.has_value(), "order creation must succeed");

        const OrderId orderId = result.value();
        const bool applied = manager.applyExecution(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::New,
            .status = OrderStatus::New,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity {}
        });

        Assert(applied, "New execution report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->exchangeOrderId == ExchangeOrderId { 1001 }, "exchange order id must be updated");
        Assert(order->status == OrderStatus::New, "invalid order status");
        Assert(order->filledQuantity.isZero(), "filled quantity must be zero");
    }

    void testApplyPartialFill()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        const OrderCreationResult result = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        Assert(result.has_value(), "order creation must succeed");

        const OrderId orderId = result.value();

        const bool applied = manager.applyExecution(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::Trade,
            .status = OrderStatus::PartiallyFilled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 40'000'000 },
            .filledQuantity = Quantity { 40'000'000 }
        });

        Assert(applied, "partial fill report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::PartiallyFilled, "order must be partially filled");
        Assert(
            order->filledQuantity == Quantity { 40'000'000 },
            "invalid filled quantity"
        );

        Assert(
            positionManager.find(InstrumentId { 1 }) == nullptr,
            "OrderManager must not modify position"
        );
    }

    void testApplyFilled()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;
        OrderManager manager { gateway, riskManager, positionManager };

        const OrderCreationResult result = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        Assert(result.has_value(), "order creation must succeed");

        const OrderId orderId = result.value();

        const bool applied = manager.applyExecution(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .execType = ExecType::Trade,
            .status = OrderStatus::Filled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity { 100'000'000 }
        });

        Assert(applied, "filled report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::Filled, "order must be filled");
        Assert(order->filledQuantity == Quantity { 100'000'000 },"invalid filled quantity");
        Assert(positionManager.find(InstrumentId { 1 }) == nullptr,"OrderManager must not modify position");
    }

    void testApplyCancelled()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        const OrderCreationResult result = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        Assert(result.has_value(), "order creation must succeed");

        const OrderId orderId = result.value();

        const bool applied = manager.applyExecution(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::Cancel,
            .status = OrderStatus::Cancelled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity {}
        });

        Assert(applied, "cancel report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::Cancelled, "order must be cancelled");
    }

    void testApplyRejected()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        const OrderCreationResult result = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        Assert(result.has_value(), "order creation must succeed");

        const OrderId orderId = result.value();

        const bool applied = manager.applyExecution(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::Reject,
            .status = OrderStatus::Rejected,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity {}
        });

        Assert(applied, "reject report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::Rejected, "order must be rejected");
    }

    void testUnknownExecutionReport()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        const bool applied = manager.applyExecution(ExecutionReport {
            .clientOrderId = OrderId { 42 },
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::Trade,
            .status = OrderStatus::Filled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity { 100'000'000 }
        });

        Assert(!applied, "report for unknown order must be rejected");
        Assert(positionManager.find(InstrumentId { 1 }) == nullptr,"unknown execution must not modify position");
    }

    void testCancelOrder()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        const OrderCreationResult result = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        Assert(result.has_value(), "order creation must succeed");

        const OrderId orderId = result.value();

        const bool cancelled = manager.cancel(orderId);

        Assert(cancelled, "cancel must succeed");
        Assert(gateway.cancelCountValue() == 1,"gateway cancel must be called once");
        Assert(gateway.cancelledOrderIdValue() == orderId,"gateway must receive correct order id");
    }

    void testCancelUnknownOrder()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        PositionManager positionManager;

        OrderManager manager { gateway, riskManager, positionManager };

        const bool cancelled = manager.cancel(OrderId { 42 });

        Assert(!cancelled, "cancel of unknown order must fail");
        Assert(gateway.cancelCountValue() == 0,"gateway cancel must not be called");
    }
}

void order_manager_test()
{
    testCreateOrder();
    testOrderIdsAreUnique();
    testOrderIsSentToGateway();
    testRiskManagerRejectsOrder();
    testFindUnknownOrder();
    testApplyNewExecutionReport();
    testApplyPartialFill();
    testApplyFilled();
    testApplyCancelled();
    testApplyRejected();
    testUnknownExecutionReport();
    testCancelOrder();
    testCancelUnknownOrder();

    std::cout << "All OrderManager tests: OK\n";
}