/**============================================================================
Name        : order_book_levels.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order book price levels.
============================================================================**/

/*
    OrderBookLevels represents a collection of price levels for one side of
    an order book.

    A level maps a price to the currently available quantity at that price.

        Price -> Quantity

    The same representation is used by:

        - OrderBook;
        - Snapshot;
        - market-data components that provide complete order-book state.

    The type is intentionally independent of OrderBook.

    This is important because OrderBookLevels represents market-data state,
    while OrderBook is a stateful component responsible for maintaining and
    modifying the current local order book.

    Keeping the level representation independent avoids dependencies such as:

        Snapshot -> OrderBook

    merely to access OrderBook::Levels.

    OrderBookLevels does not contain any business logic and does not know
    anything about:

        - exchanges;
        - market-data transport;
        - snapshots;
        - BookBuilder;
        - order execution;
        - strategies.

    It is only a normalized representation of price levels.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_LEVELS_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_LEVELS_HPP

#include "price.hpp"
#include "quantity.hpp"

#include <map>

namespace trading::market_data
{
    using OrderBookLevels = std::map<Price, Quantity>;
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_LEVELS_HPP