/**============================================================================
Name        : strategy_module_test.cpp
Created on  : 08.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : strategy_module_test.cpp
============================================================================**/

#include "strategy_module.hpp"
#include "condition_variable_queue.hpp"
#include "test_support/testing.hpp"

#include <iostream>
#include <variant>

using trading::InstrumentId;
using trading::OrderType;
using trading::Price;
using trading::Quantity;
using trading::SequenceNumber;
using trading::Side;
using trading::concurrency::ConditionVariableQueue;
using trading::execution::ExecutionWorkItem;
using trading::execution::OrderRequest;
using trading::market_data::MarketEvent;
using trading::strategy::StrategyModule;

namespace
{
    using testing::Assert;

    constexpr InstrumentId Instrument { 42 };

    constexpr Quantity OrderQuantity { 100'000'000 };

    constexpr int64_t ThresholdNumerator { 7 };
    constexpr int64_t ThresholdDenominator { 10 };

    constexpr Price BidPrice { 6'400'000'000'000 };
    constexpr Price AskPrice { 6'500'000'000'000 };

    [[nodiscard]]
    MarketEvent createBuySignalEvent()
    {
        return MarketEvent {
            .instrument = Instrument,
            .sequence = 1,
            .bestBid = BidPrice,
            .bestBidQuantity = Quantity { 100 },
            .bestAsk = AskPrice,
            .bestAskQuantity = Quantity { 10 }
        };
    }

    [[nodiscard]]
    MarketEvent createSellSignalEvent()
    {
        return MarketEvent {
            .instrument = Instrument,
            .sequence = 1,
            .bestBid = BidPrice,
            .bestBidQuantity = Quantity { 10 },
            .bestAsk = AskPrice,
            .bestAskQuantity = Quantity { 100 }
        };
    }

    [[nodiscard]]
    MarketEvent createNoneSignalEvent()
    {
        return MarketEvent {
            .instrument = Instrument,
            .sequence = 1,
            .bestBid = BidPrice,
            .bestBidQuantity = Quantity { 55 },
            .bestAsk = AskPrice,
            .bestAskQuantity = Quantity { 45 }
        };
    }

    void testBuySignalProducesBuyOrder()
    {
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;

        StrategyModule module {
            trading::config::StrategyConfig {
                .orderQuantity = OrderQuantity,
                .thresholdNumerator = ThresholdNumerator,
                .thresholdDenominator = ThresholdDenominator
            },
            marketEventQueue,
            executionQueue
        };

        marketEventQueue.push(createBuySignalEvent());
        marketEventQueue.close();

        module.run();

        ExecutionWorkItem workItem {};

        Assert(executionQueue.tryPop(workItem), "buy signal must produce execution work item");
        Assert(std::holds_alternative<OrderRequest>(workItem),"execution work item must contain OrderRequest");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.instrument == Instrument, "invalid instrument");
        Assert(request.side == Side::Buy, "buy signal must produce Buy order");
        Assert(request.type == OrderType::Limit, "strategy order must be Limit");
        Assert(request.price == AskPrice, "Buy order must use best ask price");
        Assert(request.quantity == OrderQuantity, "invalid order quantity");

        Assert(executionQueue.empty(), "only one execution work item must be produced");
    }

    void testSellSignalProducesSellOrder()
    {
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;

        StrategyModule module {
            trading::config::StrategyConfig {
                .orderQuantity = OrderQuantity,
                .thresholdNumerator = ThresholdNumerator,
                .thresholdDenominator = ThresholdDenominator
            },
            marketEventQueue,
            executionQueue
        };

        marketEventQueue.push(createSellSignalEvent());
        marketEventQueue.close();

        module.run();

        ExecutionWorkItem workItem {};

        Assert(executionQueue.tryPop(workItem), "sell signal must produce execution work item");
        Assert(std::holds_alternative<OrderRequest>(workItem),"execution work item must contain OrderRequest");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.instrument == Instrument, "invalid instrument");
        Assert(request.side == Side::Sell, "sell signal must produce Sell order");
        Assert(request.type == OrderType::Limit, "strategy order must be Limit");
        Assert(request.price == BidPrice, "Sell order must use best bid price");
        Assert(request.quantity == OrderQuantity, "invalid order quantity");

        Assert(executionQueue.empty(), "only one execution work item must be produced");
    }

    void testNoneSignalProducesNoOrder()
    {
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;

        StrategyModule module {
            trading::config::StrategyConfig {
                .orderQuantity = OrderQuantity,
                .thresholdNumerator = ThresholdNumerator,
                .thresholdDenominator = ThresholdDenominator
            },
            marketEventQueue,
            executionQueue
        };

        marketEventQueue.push(createNoneSignalEvent());
        marketEventQueue.close();

        module.run();

        ExecutionWorkItem workItem {};

        Assert(!executionQueue.tryPop(workItem), "None signal must not produce execution work item");
        Assert(executionQueue.empty(), "execution queue must remain empty");
    }

    void testInstrumentIsPropagated()
    {
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;

        StrategyModule module {
            trading::config::StrategyConfig {
                .orderQuantity = OrderQuantity,
                .thresholdNumerator = ThresholdNumerator,
                .thresholdDenominator = ThresholdDenominator
            },
            marketEventQueue,
            executionQueue
        };

        constexpr InstrumentId OtherInstrument { 123 };

        MarketEvent event = createBuySignalEvent();
        event.instrument = OtherInstrument;

        marketEventQueue.push(event);
        marketEventQueue.close();

        module.run();

        ExecutionWorkItem workItem {};
        Assert(executionQueue.tryPop(workItem), "buy signal must produce execution work item");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.instrument == OtherInstrument,"instrument must be propagated from market event");
    }

    void testConfiguredOrderQuantityIsUsed()
    {
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;

        constexpr Quantity ConfiguredQuantity { 250'000'000 };

        StrategyModule module {
            trading::config::StrategyConfig {
                .orderQuantity = ConfiguredQuantity,
                .thresholdNumerator = ThresholdNumerator,
                .thresholdDenominator = ThresholdDenominator
            },
            marketEventQueue,
            executionQueue
        };

        marketEventQueue.push(createBuySignalEvent());
        marketEventQueue.close();

        module.run();

        ExecutionWorkItem workItem {};
        Assert(executionQueue.tryPop(workItem), "buy signal must produce execution work item");

        const OrderRequest& request = std::get<OrderRequest>(workItem);
        Assert(request.quantity == ConfiguredQuantity,"configured order quantity must be used");
    }

    void testBuyUsesBestAsk()
    {
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;

        StrategyModule module {
            trading::config::StrategyConfig {
                .orderQuantity = OrderQuantity,
                .thresholdNumerator = ThresholdNumerator,
                .thresholdDenominator = ThresholdDenominator
            },
            marketEventQueue,
            executionQueue
        };

        constexpr Price DifferentBidPrice { 6'400'000'000'000 };
        constexpr Price DifferentAskPrice { 6'600'000'000'000 };

        MarketEvent event = createBuySignalEvent();
        event.bestBid = DifferentBidPrice;
        event.bestAsk = DifferentAskPrice;

        marketEventQueue.push(event);
        marketEventQueue.close();

        module.run();

        ExecutionWorkItem workItem {};
        Assert(executionQueue.tryPop(workItem), "buy signal must produce execution work item");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.price == DifferentAskPrice,"Buy order must use current best ask");
    }


    void testSellUsesBestBid()
    {
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;

        StrategyModule module {
            trading::config::StrategyConfig {
                .orderQuantity = OrderQuantity,
                .thresholdNumerator = ThresholdNumerator,
                .thresholdDenominator = ThresholdDenominator
            },
            marketEventQueue,
            executionQueue
        };

        constexpr Price DifferentBidPrice { 6'400'000'000'000 };
        constexpr Price DifferentAskPrice { 6'600'000'000'000 };

        MarketEvent event = createSellSignalEvent();
        event.bestBid = DifferentBidPrice;
        event.bestAsk = DifferentAskPrice;

        marketEventQueue.push(event);
        marketEventQueue.close();

        module.run();

        ExecutionWorkItem workItem {};
        Assert(executionQueue.tryPop(workItem), "sell signal must produce execution work item");

        const OrderRequest& request = std::get<OrderRequest>(workItem);

        Assert(request.price == DifferentBidPrice,"Sell order must use current best bid");
    }

    void testMultipleMarketEventsProduceMultipleOrders()
    {
        ConditionVariableQueue<MarketEvent> marketEventQueue;
        ConditionVariableQueue<ExecutionWorkItem> executionQueue;

        StrategyModule module {
            trading::config::StrategyConfig {
                .orderQuantity = OrderQuantity,
                .thresholdNumerator = ThresholdNumerator,
                .thresholdDenominator = ThresholdDenominator
            },
            marketEventQueue,
            executionQueue
        };

        constexpr InstrumentId FirstInstrument { 1 };
        constexpr InstrumentId SecondInstrument { 2 };

        MarketEvent buyEvent = createBuySignalEvent();
        buyEvent.instrument = FirstInstrument;

        MarketEvent sellEvent = createSellSignalEvent();
        sellEvent.instrument = SecondInstrument;

        marketEventQueue.push(buyEvent);
        marketEventQueue.push(sellEvent);
        marketEventQueue.close();

        module.run();

        Assert(!executionQueue.empty(), "execution queue must contain generated orders");

        ExecutionWorkItem firstWorkItem {};
        ExecutionWorkItem secondWorkItem {};

        Assert(executionQueue.tryPop(firstWorkItem),"first market event must produce execution work item");
        Assert(executionQueue.tryPop(secondWorkItem),"second market event must produce execution work item");
        Assert(std::holds_alternative<OrderRequest>(firstWorkItem),"first work item must contain OrderRequest");
        Assert(std::holds_alternative<OrderRequest>(secondWorkItem),"second work item must contain OrderRequest");

        const OrderRequest& firstRequest = std::get<OrderRequest>(firstWorkItem);
        const OrderRequest& secondRequest = std::get<OrderRequest>(secondWorkItem);

        Assert(firstRequest.instrument == FirstInstrument,"first order must belong to first instrument");
        Assert(firstRequest.side == Side::Buy,"first order must be Buy");
        Assert(firstRequest.price == AskPrice,"first Buy order must use ask price");
        Assert(secondRequest.instrument == SecondInstrument,"second order must belong to second instrument");
        Assert(secondRequest.side == Side::Sell,"second order must be Sell");
        Assert(secondRequest.price == BidPrice,"second Sell order must use bid price");
        Assert(executionQueue.empty(),"all generated execution work items must be consumed");
    }
}


void strategy_module_test()
{
    testBuySignalProducesBuyOrder();
    testSellSignalProducesSellOrder();
    testNoneSignalProducesNoOrder();
    testInstrumentIsPropagated();
    testConfiguredOrderQuantityIsUsed();
    testBuyUsesBestAsk();
    testSellUsesBestBid();
    testMultipleMarketEventsProduceMultipleOrders();

    std::cout << "All StrategyModule tests: OK\n";
}