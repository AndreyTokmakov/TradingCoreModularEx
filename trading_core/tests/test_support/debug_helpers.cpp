/**============================================================================
Name        : debug_helpers.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Testing and Debuging
============================================================================**/

#include "debug_helpers.hpp"
#include "model/market_event.hpp"

namespace trading::testing
{
    [[nodiscard]]
    constexpr std::string_view toString(const Side side) noexcept
    {
        switch (side)
        {
            case Side::Buy:
                return "Buy";
            case Side::Sell:
                return "Sell";
        }
        return "Unknown";
    }

    [[nodiscard]]
    constexpr std::string_view toString(const OrderType type) noexcept
    {
        switch (type)
        {
            case OrderType::Market:
                return "Market";
            case OrderType::Limit:
                return "Limit";
        }
        return "Unknown";
    }

    [[nodiscard]]
    constexpr std::string_view toString(const OrderStatus status) noexcept
    {
        switch (status)
        {
            case OrderStatus::New:
                return "New";
            case OrderStatus::PartiallyFilled:
                return "PartiallyFilled";
            case OrderStatus::Filled:
                return "Filled";
            case OrderStatus::Cancelled:
                return "Cancelled";
            case OrderStatus::Rejected:
                return "Rejected";
        }
        return "Unknown";
    }

    [[nodiscard]]
    constexpr std::string_view toString(const ExecType type) noexcept
    {
        switch (type)
        {
            case ExecType::New:
                return "New";
            case ExecType::Trade:
                return "Trade";
            case ExecType::Cancel:
                return "Cancel";
            case ExecType::Reject:
                return "Reject";
        }
        return "Unknown";
    }

    [[nodiscard]]
    constexpr std::string_view toString(const strategy::Signal signal) noexcept
    {
        switch (signal)
        {
            case strategy::Signal::None:
                return "None";
            case strategy::Signal::Buy:
                return "Buy";
            case strategy::Signal::Sell:
                return "Sell";
        }
        return "Unknown";
    }

    [[nodiscard]]
    constexpr std::string_view toString(const market_data::ParseResult result) noexcept
    {
        switch (result)
        {
            case market_data::ParseResult::Success:
                return "Success";
            case market_data::ParseResult::EmptyMessage:
                return "EmptyMessage";
            case market_data::ParseResult::InvalidMessage:
                return "InvalidMessage";
            case market_data::ParseResult::InvalidSide:
                return "InvalidSide";
            case market_data::ParseResult::UnsupportedMessage:
                return "UnsupportedMessage";
            case market_data::ParseResult::MissingField:
                return "MissingField";
            case market_data::ParseResult::InvalidField:
                return "InvalidField";
            case market_data::ParseResult::InvalidInstrument:
                return "InvalidInstrument";
            case market_data::ParseResult::InvalidSequence:
                return "InvalidSequence";
            case market_data::ParseResult::InvalidTimestamp:
                return "InvalidTimestamp";
            case market_data::ParseResult::InvalidPrice:
                return "InvalidPrice";
            case market_data::ParseResult::InvalidQuantity:
                return "InvalidQuantity";
        }
        return "Unknown";
    }

    std::ostream& operator<<(std::ostream& stream, const Side side)
    {
        return stream << toString(side);
    }

    std::ostream& operator<<(std::ostream& stream, const OrderType type)
    {
        return stream << toString(type);
    }

    std::ostream& operator<<(std::ostream& stream, const OrderStatus status)
    {
        return stream << toString(status);
    }

    std::ostream& operator<<(std::ostream& stream, const ExecType type)
    {
        return stream << toString(type);
    }

    std::ostream& operator<<(std::ostream& stream, const market_data::ParseResult result)
    {
        return stream << toString(result);
    }

    std::ostream& operator<<(std::ostream& stream, const strategy::Signal signal)
    {
        return stream << toString(signal);
    }

    std::ostream& operator<<(std::ostream& stream, const execution::OrderRequest& request)
    {
        return stream << "OrderRequest {"
                      << "\n\t instrument=" << request.instrument
                      << "\n\t side=" << request.side
                      << "\n\t type=" << request.type
                      << "\n\t price=" << request.price.raw()
                      << "\n\t quantity=" << request.quantity.raw()
                      << "\n}\n";
    }

    std::ostream& operator<<(std::ostream& stream, const execution::Order& order)
    {
        return stream << "Order {"
                      << "\n\t clientOrderId=" << order.clientOrderId
                      << "\n\t exchangeOrderId=" << order.exchangeOrderId
                      << "\n\t instrument=" << order.instrument
                      << "\n\t side=" << order.side
                      << "\n\t type=" << order.type
                      << "\n\t price=" << order.price.raw()
                      << "\n\t quantity=" << order.quantity.raw()
                      << "\n\t filledQuantity=" << order.filledQuantity.raw()
                      << "\n\t status=" << order.status
                      << "\n}\n";
    }

    std::ostream& operator<<(std::ostream& stream, const execution::ExecutionReport& report)
    {
        return stream << "ExecutionReport {"
                      << "\n\t clientOrderId=" << report.clientOrderId
                      << "\n\t exchangeOrderId=" << report.exchangeOrderId
                      << "\n\t instrument=" << report.instrument
                      << "\n\t side=" << report.side
                      << "\n\t execType=" << report.execType
                      << "\n\t status=" << report.status
                      << "\n\t price=" << report.price.raw()
                      << "\n\t quantity=" << report.quantity.raw()
                      << "\n\t filledQuantity=" << report.filledQuantity.raw()
                      << "\n}\n";
    }

    std::ostream& operator<<(std::ostream& stream, const market_data::BookUpdate& update)
    {
        return stream << "BookUpdate {"
                      << "\n\t instrument=" << update.instrument
                      << "\n\t sequence=" << update.sequence
                      << "\n\t exchangeTimestamp=" << update.exchangeTimestamp.nanoseconds()
                      << "\n\t side=" << update.side
                      << "\n\t price=" << update.price.raw()
                      << "\n\t quantity=" << update.quantity.raw()
                      << "\n}\n";
    }

    std::ostream& operator<<(std::ostream& stream, const market_data::MarketEvent& event)
    {
        return stream << "MarketEvent {"
                      << "\n\t instrument=" << event.instrument
                      << "\n\t sequence=" << event.sequence
                      << "\n\t exchangeTimestamp=" << event.exchangeTimestamp.nanoseconds()
                      << "\n\t receiveTimestamp=" << event.receiveTimestamp.nanoseconds()
                      << "\n\t bestBid=" << event.bestBid.raw()
                      << "\n\t bestBidQuantity=" << event.bestBidQuantity.raw()
                      << "\n\t bestAsk=" << event.bestAsk.raw()
                      << "\n\t bestAskQuantity=" << event.bestAskQuantity.raw()
                      << "\n}\n";
    }

    void printMarketEvent(const market_data::MarketEvent &event)
    {
        std::cout << "MarketEvent {"
            << "\n\t instrument       : " << event.instrument
            << "\n\t sequence         : " << event.sequence
            << "\n\t exchangeTimestamp: " << event.exchangeTimestamp.nanoseconds()
            << "\n\t receiveTimestamp : " << event.receiveTimestamp .nanoseconds()
            << "\n\t bestBid          : " << event.bestBid.raw()
            << "\n\t bestBidQuantity  : " << event.bestBidQuantity.raw()
            << "\n\t bestAsk          : " << event.bestAsk.raw()
            << "\n\t bestAskQuantity  : " << event.bestAskQuantity.raw()
            << "\n}\n";
    }
}

