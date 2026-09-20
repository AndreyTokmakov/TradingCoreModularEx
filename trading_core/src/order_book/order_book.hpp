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

#include <flat_map>
#include <optional>

namespace trading::order_book
{
    using market_data::BookUpdate;
    using market_data::BookLevel;
    using market_data::OrderBookLevels;

    class OrderBook
    {
    public:
        using Levels    = std::map<Price, Quantity>;
        // using Levels    = std::vector<std::pair<Price, Quantity>>;

        using BidLevels = std::flat_map<Price, Quantity, std::greater<>>;
        using AskLevels = std::flat_map<Price, Quantity, std::less<>>;
        using size_type = size_t;

        static constexpr size_type DefaultDepthValue { 500 };

        explicit OrderBook(const size_type depth = DefaultDepthValue) noexcept :
            depthValue { depth }
        {
        }

        OrderBook(const OrderBook&) = default;
        OrderBook& operator=(const OrderBook&) = default;

        OrderBook(OrderBook&&) noexcept = default;
        OrderBook& operator=(OrderBook&&) noexcept = default;

        [[nodiscard]]
        SequenceNumber sequence() const noexcept;

        void clear() noexcept;

        void setState(SequenceNumber sequence,
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

        [[nodiscard]]
        size_type bidSize() const noexcept;

        [[nodiscard]]
        size_type askSize() const noexcept;

    private:

        size_type depthValue { 0 };
        SequenceNumber sequenceNumber { 0 };

        BidLevels bids;
        AskLevels asks;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_HPP