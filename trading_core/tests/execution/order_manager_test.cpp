#include "order_manager.hpp"

#include "risk_manager.hpp"
#include "test_support/testing.hpp"

#include <iostream>

using trading::ExchangeOrderId;
using trading::ExecType;
using trading::InstrumentId;
using trading::OrderId;
using trading::OrderStatus;
using trading::OrderType;
using trading::Price;
using trading::Quantity;
using trading::Side;

using trading::execution::ExecutionReport;
using trading::execution::IExecutionGateway;
using trading::execution::Order;
using trading::execution::OrderCreationError;
using trading::execution::OrderCreationResult;
using trading::execution::OrderManager;
using trading::execution::OrderRequest;

using trading::position::Position;
using trading::position::PositionManager;

using trading::risk::RiskLimits;
using trading::risk::RiskManager;

namespace
{
    using testing::Assert;

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

    constexpr InstrumentId INSTRUMENT { 1 };
    constexpr Price PRICE { 6500000000000 };
    constexpr Quantity QUANTITY { 100000000 };

    RiskManager createRiskManager()
    {
        return RiskManager { RiskLimits {} };
    }

    OrderRequest makeOrderRequest()
    {
        return OrderRequest {
            .instrument = INSTRUMENT,
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = PRICE,
            .quantity = QUANTITY
        };
    }

    ExecutionReport makeExecutionReport(const OrderId orderId,
                                        const ExecType execType,
                                        const OrderStatus status,
                                        const Quantity filledQuantity,
                                        const Side side = Side::Buy)
    {
        return ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 123 },
            .instrument = INSTRUMENT,
            .side = side,
            .execType = execType,
            .status = status,
            .price = PRICE,
            .quantity = QUANTITY,
            .filledQuantity = filledQuantity
        };
    }

    void testCreateOrder()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderCreationResult result = manager.createOrder(request);

        Assert(result.has_value(), "order creation must succeed");

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
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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
        Assert(third.value() == 3, "third order id must be three");

        Assert(manager.find(first.value()) != nullptr, "first order must exist");
        Assert(manager.find(second.value()) != nullptr, "second order must exist");
        Assert(manager.find(third.value()) != nullptr, "third order must exist");
    }

    void testOrderIsSentToGateway()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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

        RiskLimits riskLimits {};
        riskLimits.maxOrderQuantity = Quantity { 50'000'000 };

        RiskManager riskManager { riskLimits };
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderCreationResult result = manager.createOrder(request);

        Assert(!result.has_value(), "risk rejected order must not be created");
        Assert(result.error() == OrderCreationError::RiskRejected,"invalid order creation error");
        Assert(gateway.sendCountValue() == 0, "risk rejected order must not be sent");
        Assert(manager.find(OrderId { 1 }) == nullptr, "risk rejected order must not exist");
    }

    void testInvalidOrderRequest()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 0 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderCreationResult result = manager.createOrder(request);

        Assert(!result.has_value(), "invalid order must not be created");
        Assert(result.error() == OrderCreationError::InvalidRequest,"invalid order creation error");
        Assert(gateway.sendCountValue() == 0, "invalid order must not be sent");
    }

    void testInvalidPriceIsRejected()
    {
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;
        TestExecutionGateway gateway;
        OrderManager manager { riskManager, positionManager, gateway };

        OrderRequest request = makeOrderRequest();
        request.price = Price { 0 };

        const OrderCreationResult result = manager.createOrder(request);

        Assert(!result.has_value(), "Zero price should reject the order");
        Assert(result.error() == OrderCreationError::InvalidRequest, "Wrong error for zero price");
        Assert(gateway.sendCountValue() == 0, "Invalid order should not be sent to gateway");
    }

    void testInvalidQuantityIsRejected()
    {
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;
        TestExecutionGateway gateway;
        OrderManager orderManager { riskManager, positionManager, gateway };

        OrderRequest request = makeOrderRequest();
        request.quantity = Quantity { 0 };

        const auto result = orderManager.createOrder(request);

        Assert(!result.has_value(), "Zero quantity should reject the order");
        Assert(result.error() == OrderCreationError::InvalidRequest, "Wrong error for zero quantity");
        Assert(gateway.sendCountValue() == 0, "Invalid order should not be sent to gateway");
    }

    void testFindUnknownOrder()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

        const Order* order = manager.find(OrderId { 42 });
        Assert(order == nullptr, "unknown order must not be found");
    }

    void testApplyNewExecutionReport()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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
        Assert(positionManager.find(InstrumentId { 1 }) == nullptr,"New execution must not modify position");
    }

    void testApplyPartialFill()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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
        Assert(order->filledQuantity == Quantity { 40'000'000 }, "invalid filled quantity");

        const Position* position = positionManager.find(InstrumentId { 1 });

        Assert(position != nullptr, "partial fill must create position");
        Assert(position->quantity() == 40'000'000, "invalid position quantity after partial fill");
        Assert(
            position->averagePrice() == Price { 6'500'000'000'000 },
            "invalid position average price after partial fill");
    }

    void testApplyMultiplePartialFills()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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

        const bool firstApplied = manager.applyExecution(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::Trade,
            .status = OrderStatus::PartiallyFilled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 30'000'000 },
            .filledQuantity = Quantity { 30'000'000 }
        });

        Assert(firstApplied, "first partial fill must be applied");

        const bool secondApplied = manager.applyExecution(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::Trade,
            .status = OrderStatus::PartiallyFilled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 40'000'000 },
            .filledQuantity = Quantity { 70'000'000 }
        });

        Assert(secondApplied, "second partial fill must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::PartiallyFilled, "order must remain partially filled");
        Assert(
            order->filledQuantity == Quantity { 70'000'000 },
            "filled quantity must contain cumulative quantity");

        const Position* position = positionManager.find(InstrumentId { 1 });

        Assert(position != nullptr, "position must exist");
        Assert(
            position->quantity() == 70'000'000,
            "position quantity must contain sum of individual fills");
    }

    void testApplyFilled()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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
            .status = OrderStatus::Filled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity { 100'000'000 }
        });

        Assert(applied, "filled report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::Filled, "order must be filled");
        Assert(order->filledQuantity == Quantity { 100'000'000 }, "invalid filled quantity");

        const Position* position = positionManager.find(InstrumentId { 1 });

        Assert(position != nullptr, "filled execution must create position");
        Assert(position->quantity() == 100'000'000, "invalid position quantity");
        Assert(
            position->averagePrice() == Price { 6'500'000'000'000 },
            "invalid position average price");
    }

    void testApplyCancelled()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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
        Assert(
            positionManager.find(InstrumentId { 1 }) == nullptr,
            "cancel execution must not modify position");
    }

    void testApplyRejected()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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
        Assert(
            positionManager.find(InstrumentId { 1 }) == nullptr,
            "reject execution must not modify position");
    }

    void testUnknownExecutionReport()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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
        Assert(
            positionManager.find(InstrumentId { 1 }) == nullptr,
            "unknown execution must not modify position");
    }

    void testCancelOrder()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

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
        Assert(gateway.cancelCountValue() == 1, "gateway cancel must be called once");
        Assert(gateway.cancelledOrderIdValue() == orderId, "gateway must receive correct order id");
    }

    void testCancelUnknownOrder()
    {
        TestExecutionGateway gateway;
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;

        OrderManager manager { riskManager, positionManager, gateway };

        const bool cancelled = manager.cancel(OrderId { 42 });

        Assert(!cancelled, "cancel of unknown order must fail");
        Assert(gateway.cancelCountValue() == 0, "gateway cancel must not be called");
    }

    void testExecutionUpdatesOrderFromPartiallyFilledToFilled()
    {
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;
        TestExecutionGateway gateway;
        OrderManager orderManager { riskManager, positionManager, gateway };

        const OrderCreationResult result = orderManager.createOrder(makeOrderRequest());
        Assert(result.has_value(), "Order creation should succeed");

        const OrderId orderId = result.value();
        const ExecutionReport partialReport = makeExecutionReport(orderId,
                                ExecType::Trade,
                                OrderStatus::PartiallyFilled,
                                Quantity { 40 });

        const ExecutionReport filledReport = makeExecutionReport(orderId,
                                ExecType::Trade,
                                OrderStatus::Filled,
                                QUANTITY);

        Assert(orderManager.applyExecution(partialReport), "Partial fill should be applied");
        Assert(orderManager.applyExecution(filledReport), "Filled report should be applied");

        const Order* order = orderManager.find(orderId);

        Assert(order != nullptr, "Order should exist");
        Assert(order->status == OrderStatus::Filled, "Order should be Filled");
        Assert(order->filledQuantity == QUANTITY, "Filled quantity mismatch");

        const Position* position = positionManager.find(INSTRUMENT);

        Assert(position != nullptr, "Position should exist");
        Assert(position->quantity() == QUANTITY.raw(), "Position quantity mismatch");
    }

    void testSellExecutionUpdatesPosition()
    {
        RiskManager riskManager = createRiskManager();
        PositionManager positionManager;
        TestExecutionGateway gateway;
        OrderManager orderManager { riskManager, positionManager, gateway };

        OrderRequest request = makeOrderRequest();
        request.side = Side::Sell;

        const OrderCreationResult result = orderManager.createOrder(request);
        Assert(result.has_value(), "Sell order creation should succeed");

        const ExecutionReport report = makeExecutionReport(result.value(),
                                ExecType::Trade,
                                OrderStatus::Filled,
                                QUANTITY,
                                Side::Sell);

        const bool applied = orderManager.applyExecution(report);
        Assert(applied, "Sell execution report should be applied");

        const Position* position = positionManager.find(INSTRUMENT);
        Assert(position != nullptr, "Position should exist");
        Assert(position->quantity() == (-1) * QUANTITY.raw(), "Sell execution should create short position");
        Assert(position->isShort(), "Position should be short");
    }

}

void order_manager_test()
{
    testCreateOrder();
    testOrderIdsAreUnique();
    testOrderIsSentToGateway();
    testRiskManagerRejectsOrder();

    testInvalidOrderRequest();
    testInvalidPriceIsRejected();
    testInvalidQuantityIsRejected();

    testFindUnknownOrder();

    testApplyNewExecutionReport();
    testApplyPartialFill();
    testApplyMultiplePartialFills();
    testApplyFilled();
    testApplyCancelled();
    testApplyRejected();

    testUnknownExecutionReport();
    testCancelOrder();
    testCancelUnknownOrder();

    testExecutionUpdatesOrderFromPartiallyFilledToFilled();
    testSellExecutionUpdatesPosition();

    std::cout << "All OrderManager tests: OK\n";
}