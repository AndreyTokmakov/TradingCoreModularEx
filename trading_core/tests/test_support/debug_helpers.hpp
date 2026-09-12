/**============================================================================
Name        : debug_helpers.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Testing and Debuging
============================================================================**/

#ifndef TRADINGCOREMODULAREX_DEBUG_HELPERS_HPP
#define TRADINGCOREMODULAREX_DEBUG_HELPERS_HPP

#include "model/market_event.hpp"
#include "model/parse_result.hpp"
#include "model/order.hpp"
#include "model/execution_report.hpp"
#include "model/book_level.hpp"
#include "model/order_book_levels.hpp"
#include "model/book_update.hpp"
#include "model/snapshot.hpp"
#include "signal.hpp"

#include <iostream>


namespace trading::testing
{
    [[nodiscard]]
    constexpr std::string_view toString(Side side) noexcept;

    [[nodiscard]]
    constexpr std::string_view toString(OrderType type) noexcept;

    [[nodiscard]]
    constexpr std::string_view toString(OrderStatus status) noexcept;

    [[nodiscard]]
    constexpr std::string_view toString(ExecType type) noexcept;

    [[nodiscard]]
    constexpr std::string_view toString(strategy::Signal signal) noexcept;

    [[nodiscard]]
    constexpr std::string_view toString(market_data::ParseResult result) noexcept;

    [[nodiscard]]
    std::string toString(const  execution::OrderRequest& request);

    [[nodiscard]]
    std::string toString(const execution::Order& order);

    [[nodiscard]]
    std::string toString(const execution::ExecutionReport& report);

    [[nodiscard]]
    std::string toString(const market_data::BookUpdate& update);

    [[nodiscard]]
    std::string toString(const market_data::MarketEvent& event);

    [[nodiscard]]
    std::string toString(const market_data::BookLevel& level);

    [[nodiscard]]
    std::string toString(const market_data::OrderBookLevels& levels);

    [[nodiscard]]
    std::string toString(const market_data::Snapshot& snapshot);

    std::ostream& operator<<(std::ostream& stream, Side side);
    std::ostream& operator<<(std::ostream& stream, OrderType type);
    std::ostream& operator<<(std::ostream& stream, OrderStatus status);
    std::ostream& operator<<(std::ostream& stream, ExecType type);
    std::ostream& operator<<(std::ostream& stream, market_data::ParseResult result);
    std::ostream& operator<<(std::ostream& stream, strategy::Signal signal);

    std::ostream& operator<<(std::ostream& stream, const execution::OrderRequest& request);
    std::ostream& operator<<(std::ostream& stream, const execution::Order& order);
    std::ostream& operator<<(std::ostream& stream, const execution::ExecutionReport& report);
    std::ostream& operator<<(std::ostream& stream, const market_data::BookUpdate& update);
    std::ostream& operator<<(std::ostream& stream, const market_data::MarketEvent& event);
    std::ostream& operator<<(std::ostream& stream, const market_data::BookLevel& level);
    std::ostream& operator<<(std::ostream& stream, const market_data::OrderBookLevels& levels);
    std::ostream& operator<<(std::ostream& stream, const market_data::Snapshot& snapshot);
}

#endif //TRADINGCOREMODULAREX_DEBUG_HELPERS_HPP