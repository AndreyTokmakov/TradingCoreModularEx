/**============================================================================
Name        : order_book.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order book state management.
============================================================================**/

#include "order_book.hpp"

namespace trading::order_book
{
    template<typename Levels>
    [[nodiscard]]
    static bool applyBookUpdate(Levels& levels,
                                const BookUpdate& update,
                                const OrderBook::size_type depth) noexcept
    {
        if (update.quantity.isZero())
            return levels.erase(update.price) != 0;

        if (const auto iterator = levels.find(update.price); iterator != levels.end()) {
            iterator->second = update.quantity;
            return true;
        }

        if (levels.size() < depth) {
            levels.emplace(update.price, update.quantity);
            return true;
        }

        const auto itWorst = std::prev(levels.end());
        if (!levels.key_comp()(update.price, itWorst->first))
            return false;

        levels.erase(itWorst);
        levels.emplace(update.price, update.quantity);

        return true;
    }

    template<typename Levels>
    [[nodiscard]]
    static Quantity getVolume(const Levels& levels,
                              const Price price) noexcept
    {
        const auto iterator = levels.find(price);
        if (iterator == levels.end())
            return Quantity{0};

        return iterator->second;
    }

    template<typename Levels>
    [[nodiscard]]
    static std::optional<BookLevel> getBestPrice(const Levels& levels)
    {
        if (levels.empty())
            return std::nullopt;

        const auto& [price, quantity] = *levels.begin();
        return BookLevel {
            .price = price,
            .quantity = quantity
        };
    }
}

namespace trading::order_book
{
    SequenceNumber OrderBook::sequence() const noexcept
    {
        return sequenceNumber;
    }

    void OrderBook::clear() noexcept
    {
        bids.clear();
        asks.clear();
        sequenceNumber = 0;
    }

    void OrderBook::setState(const SequenceNumber sequence,
                             const Levels& snapBids,
                             const Levels& snapAsks)
    {
        bids = BidLevels{snapBids.cbegin(), snapBids.cend()};
        asks = AskLevels{snapAsks.cbegin(), snapAsks.cend()};
        sequenceNumber = sequence;
    }

    bool OrderBook::applyUpdate(const BookUpdate& update) noexcept
    {
        if (update.sequence != sequenceNumber + 1){
            return false;
        }
        const bool result = Side::Buy == update.side?
            applyBookUpdate(bids, update, depthValue) :
            applyBookUpdate(asks, update, depthValue);
        if (!result) {
            return false;
        }

        sequenceNumber = update.sequence;
        return true;
    }

    std::optional<BookLevel> OrderBook::bestBid() const {
        return getBestPrice(bids);
    }

    std::optional<BookLevel> OrderBook::bestAsk() const {
        return getBestPrice(asks);
    }

    Quantity OrderBook::bidVolume(const Price price) const noexcept {
        return getVolume(bids, price);
    }

    Quantity OrderBook::askVolume(const Price price) const noexcept {
        return getVolume(asks, price);
    }

    OrderBook::size_type OrderBook::bidSize() const noexcept {
        return bids.size();
    }

    OrderBook::size_type OrderBook::askSize() const noexcept {
        return asks.size();
    }
}