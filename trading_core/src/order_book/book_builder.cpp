/**============================================================================
Name        : book_builder.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_builder.cpp
============================================================================**/

/*
    BookBuilder receives normalized BookUpdate events, applies them to the
    associated OrderBook and publishes a MarketEvent containing the resulting
    top-of-book state.

    Data Flow:

        BookUpdate
            |
            v
        BookBuilder
            |
            +----> validate instrument
            |
            +----> apply update
            |          |
            |          v
            |      OrderBook
            |
            v
        MarketEvent
            |
            v
        IMarketEventHandler

    An update that belongs to another instrument or cannot be applied because
    the OrderBook is invalid or the sequence is incorrect is ignored and does
    not produce a MarketEvent.
*/

#include "book_builder.hpp"

#include <iostream>

namespace trading::order_book
{
    BookBuilder::BookBuilder(const InstrumentId instrument,
                             OrderBook& orderBook,
                             MarketEventDispatcher& eventDispatcher) noexcept :
        instrument { instrument },
        orderBook { orderBook },
        eventDispatcher { eventDispatcher }
    {
    }

    bool BookBuilder::applySnapshot(const Snapshot& snapshot) const
    {
        if (snapshot.instrument != instrument)
            return false;

        std::cout <<  "snapshot.sequence = " << snapshot.sequence << std::endl;
        orderBook.setState(snapshot.sequence, snapshot.bids, snapshot.asks);

        return true;
    }

    void BookBuilder::onBookUpdate(const BookUpdate& update) const
    {
        std::cout <<  "onBookUpdate (size = " << update.updates.size() << ")"<< std::endl;
        if (update.instrument != instrument)
            return;
        if (!isSequenceValid(update)) {
            std::cout << "ERROR: " << update.sequenceRange.first << " != " << orderBook.sequence() + 1 << std::endl;
            return;
        }
        if (!applyBookUpdate(update))
            return;

        orderBook.setSequence(update.sequenceRange.last);
        publishMarketEvent(update.sequenceRange.last, update.exchangeTimestamp);
    }

    bool BookBuilder::isSequenceValid(const BookUpdate& update) const noexcept
    {
        return update.sequenceRange.first == orderBook.sequence() + 1;
    }

    bool BookBuilder::applyBookUpdate(const BookUpdate& update) const noexcept
    {
        return std::ranges::all_of(update.updates, [this](const PriceLevelUpdate& levelUpdate) {
            std::cout <<  "applyBookUpdate "<< std::endl;
            return true == orderBook.applyUpdate(levelUpdate);
        });
    }

    void BookBuilder::publishMarketEvent(const SequenceNumber sequence,
                                         const Timestamp exchangeTimestamp) const
    {
        const auto bestBid = orderBook.bestBid();
        const auto bestAsk = orderBook.bestAsk();

        eventDispatcher.onMarketEvent(MarketEvent {
            .instrument = instrument,
            .sequence = sequence,
            .exchangeTimestamp = exchangeTimestamp,
            .receiveTimestamp = Timestamp::now(),
            .bestBid = bestBid ? bestBid->price : Price {},
            .bestBidQuantity = bestBid ? bestBid->quantity : Quantity {},
            .bestAsk = bestAsk ? bestAsk->price : Price {},
            .bestAskQuantity = bestAsk ? bestAsk->quantity : Quantity {}
        });
    }
}
