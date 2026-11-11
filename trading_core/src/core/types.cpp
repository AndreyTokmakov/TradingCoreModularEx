/**============================================================================
Name        : types.cpp
Created on  : 11.11.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : types.cpp
============================================================================**/

#include "types.hpp"

namespace trading
{
    std::string_view toString(const Side side) noexcept
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

    std::string_view toString(const OrderType type) noexcept
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

    std::string_view toString(const OrderStatus status) noexcept
    {
        switch (status)
        {
            case OrderStatus::PendingNew:
                return "PendingNew";
            case OrderStatus::New:
                return "New";
            case OrderStatus::PartiallyFilled:
                return "PartiallyFilled";
            case OrderStatus::Filled:
                return "Filled";
            case OrderStatus::PendingCancel:
                return "PendingCancel";
            case OrderStatus::Canceled:
                return "Canceled";
            case OrderStatus::Rejected:
                return "Rejected";
            case OrderStatus::Expired:
                return "Expired";
            case OrderStatus::Unknown:
                return "Unknown";
        }
        return "Unknown";
    }

    std::string_view toString(const ExecType type) noexcept
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
}
