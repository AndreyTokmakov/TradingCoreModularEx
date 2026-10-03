/**============================================================================
Name        : book_update.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Normalized market data update containing one or more order book level changes.
============================================================================**/

/*
    BookUpdate represents a normalized incremental update to an OrderBook.

    A BookUpdate belongs to a single financial instrument and may contain one or more
    PriceLevelUpdate entries. Each PriceLevelUpdate describes a change to one price
    level on either the bid or ask side.

    A zero quantity means that the corresponding price level must be removed from
    the OrderBook.

    BookUpdate also contains a SequenceRange identifying the range of market data
    sequence numbers covered by the update and the exchange timestamp associated
    with the update.

    BookUpdate is a normalized internal representation of exchange-specific
    incremental market data. Exchange-specific market data formats should be
    converted to BookUpdate by the corresponding market data parser before being
    passed to the BookBuilder.

    Data Flow:

        Exchange
           |
           | exchange-specific market data
           v
        MarketDataSource
           |
           v
        Parser
           |
           | BookUpdate
           v
        BookBuilder
           |
           v
        OrderBook
           |
           | updated top-of-book
           v
        MarketEvent
           |
           v
        Strategy

    BookUpdate is created by the market data layer after an exchange-specific
    message has been parsed and normalized.

    BookUpdate is consumed by BookBuilder, which validates the sequence range,
    applies all contained PriceLevelUpdate entries to the corresponding OrderBook,
    and advances the OrderBook sequence to sequenceRange.last.

    A single accepted BookUpdate produces one MarketEvent after all contained
    price level updates have been applied.

    If the sequence range is not valid, BookBuilder does not apply the update
    and does not produce a MarketEvent.

    Fields:
        instrument
            Identifier of the financial instrument whose order book is being updated.

        sequenceRange
            Range of market data sequence numbers covered by this update.

            first
                First sequence number covered by the update.

            last
                Last sequence number covered by the update.

            BookBuilder uses the sequence range to validate continuity of the
            market data stream and advances the OrderBook sequence to the last
            sequence number after successfully applying the update.

        exchangeTimestamp
            Timestamp assigned by the exchange to this market data update.

        updates
            Collection of PriceLevelUpdate entries describing the individual
            order book level changes contained in this update.

            Each entry specifies:
                side
                    Order book side affected by the update:
                        Side::Buy  - bid side
                        Side::Sell - ask side

                price
                    Price level affected by the update.

                quantity
                    New quantity available at the specified price level.

                    A non-zero quantity creates a new price level or replaces
                    the existing quantity at that level.

                    A zero quantity removes the price level from the OrderBook.

    Example:

        BookUpdate {
            instrument = 42,
            sequenceRange = { 101, 103 },
            exchangeTimestamp = ...,
            updates = {
                { Side::Buy,  bidPrice,  bidQuantity },
                { Side::Sell, askPrice, askQuantity },
                { Side::Buy,  bidPrice2, bidQuantity2 }
            }
        }

    This represents one market data update covering sequences 101 through 103
    and containing three individual price level changes.
*/


#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP

#include "core/timestamp.hpp"
#include "core/types.hpp"

#include "sequence_range.hpp"
#include "book_level_update.hpp"

namespace trading::market_data
{
    struct BookUpdate
    {
        InstrumentId instrument { 0 };
        SequenceRange sequenceRange { 0 };
        Timestamp exchangeTimestamp {};
        std::vector<PriceLevelUpdate> updates;

        void clear()
        {
            instrument = InstrumentId{ 0 };
            sequenceRange = SequenceRange { .first = 0, .last = 0 };
            exchangeTimestamp = Timestamp { 0 };
            updates.clear();
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return updates.empty();
        }
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP