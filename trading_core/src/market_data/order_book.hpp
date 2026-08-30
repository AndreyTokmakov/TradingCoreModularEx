/**============================================================================
Name        : order_book.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order book state.
              OrderBook stores current bid/ask levels and applies market-data
              updates. It does not know about snapshots or market-data sources.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_HPP

#include "model/book_level.hpp"
#include "model/book_update.hpp"
#include "model/order_book_levels.hpp"
#include "price.hpp"
#include "quantity.hpp"

#include <map>
#include <optional>

namespace trading::market_data
{
    class OrderBook
    {
    public:
        using Levels = OrderBookLevels;

        [[nodiscard]]
        SequenceNumber sequence() const noexcept;

        void clear() noexcept;

        void replace(SequenceNumber sequence,
                     const Levels& snapBids,
                     const Levels& snapAsks);

        [[nodiscard]]
        bool applyUpdate(const BookUpdate& update) noexcept;

        [[nodiscard]]
        std::optional<BookLevel> bestBid() const;

        [[nodiscard]]
        std::optional<BookLevel> bestAsk() const;

        [[nodiscard]]
        Quantity bidVolume(Price price) const noexcept;

        [[nodiscard]]
        Quantity askVolume(Price price) const noexcept;

    private:
        Levels bids;
        Levels asks;
        SequenceNumber sequenceNumber { 0 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_HPP