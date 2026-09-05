#include "order_book.hpp"
#include "test_support/testing.hpp"

#include <iostream>

namespace
{
    using trading::Price;
    using trading::Quantity;
    using trading::Side;
    using trading::market_data::BookUpdate;
    using trading::market_data::OrderBook;
    using testing::Assert;


    void testEmptyBook()
    {
        const OrderBook book;

        Assert(book.sequence() == 0, "new book sequence must be zero");
        Assert(!book.bestBid().has_value(), "empty book must have no best bid");
        Assert(!book.bestAsk().has_value(), "empty book must have no best ask");
    }

    void testReplace()
    {
        OrderBook book;

        const OrderBook::Levels bids {
            { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } },
            { Price { 6'499'999'000'000 }, Quantity { 250'000'000 } }
        };

        const OrderBook::Levels asks {
            { Price { 6'500'001'000'000 }, Quantity { 90'000'000 } },
            { Price { 6'500'002'000'000 }, Quantity { 310'000'000 } }
        };

        book.replace(100, bids, asks);

        Assert(book.sequence() == 100, "replace sequence must be stored");

        const auto bestBid = book.bestBid();

        Assert(bestBid.has_value(), "best bid must exist");
        Assert(bestBid->price == Price { 6'500'000'000'000 }, "invalid best bid price");
        Assert(bestBid->quantity == Quantity { 120'000'000 }, "invalid best bid quantity");

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "best ask must exist");
        Assert(bestAsk->price == Price { 6'500'001'000'000 }, "invalid best ask price");
        Assert(bestAsk->quantity == Quantity { 90'000'000 }, "invalid best ask quantity");
    }

    void testAddBid()
    {
        OrderBook book;

        book.replace(100, {}, {});

        const bool applied = book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 101,
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 120'000'000 }
        });

        Assert(applied, "bid update must be applied");
        Assert(book.sequence() == 101, "sequence must be updated");

        const auto bestBid = book.bestBid();

        Assert(bestBid.has_value(), "best bid must exist");
        Assert(bestBid->price == Price { 6'500'000'000'000 }, "invalid bid price");
        Assert(bestBid->quantity == Quantity { 120'000'000 }, "invalid bid quantity");
    }

    void testAddAsk()
    {
        OrderBook book;

        book.replace(100, {}, {});

        const bool applied = book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 101,
            .side = Side::Sell,
            .price = Price { 6'500'001'000'000 },
            .quantity = Quantity { 90'000'000 }
        });

        Assert(applied, "ask update must be applied");
        Assert(book.sequence() == 101, "sequence must be updated");

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "best ask must exist");
        Assert(bestAsk->price == Price { 6'500'001'000'000 }, "invalid ask price");
        Assert(bestAsk->quantity == Quantity { 90'000'000 }, "invalid ask quantity");
    }

    void testUpdateExistingLevel()
    {
        OrderBook book;

        book.replace(
            100,
            {
                { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }
            },
            {});

        const bool applied = book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 101,
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 200'000'000 }
        });

        Assert(applied, "level update must be applied");
        Assert(
            book.bidVolume(Price { 6'500'000'000'000 }) == Quantity { 200'000'000 },
            "level quantity must be updated");
    }

    void testRemoveLevel()
    {
        OrderBook book;

        book.replace(
            100,
            {
                { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }
            },
            {});

        const bool applied = book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 101,
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity {}
        });

        Assert(applied, "remove update must be applied");
        Assert(book.bidVolume(Price { 6'500'000'000'000 }).isZero(), "removed level must have zero volume");
        Assert(!book.bestBid().has_value(), "book must have no best bid");
    }

    void testRemoveUnknownLevel()
    {
        OrderBook book;

        constexpr Price existingPrice { 6'500'000'000'000 };
        constexpr Price unknownPrice { 6'499'999'000'000 };

        book.replace(
            100,
            {
                { existingPrice, Quantity { 120'000'000 } }
            },
            {});

        const bool applied = book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 101,
            .side = Side::Buy,
            .price = unknownPrice,
            .quantity = Quantity {}
        });

        Assert(applied, "remove of unknown level must be applied");
        Assert(book.sequence() == 101, "sequence must be updated");
        Assert(book.bidVolume(existingPrice) == Quantity { 120'000'000 }, "existing level must not be modified");
        Assert(book.bidVolume(unknownPrice).isZero(), "unknown level must remain absent");
    }

    void testBestBid()
    {
        OrderBook book;

        book.replace(
            100,
            {
                { Price { 6'499'999'000'000 }, Quantity { 100'000'000 } },
                { Price { 6'500'000'000'000 }, Quantity { 200'000'000 } },
                { Price { 6'499'998'000'000 }, Quantity { 300'000'000 } }
            },
            {});

        const auto bestBid = book.bestBid();

        Assert(bestBid.has_value(), "best bid must exist");
        Assert(bestBid->price == Price { 6'500'000'000'000 }, "invalid best bid");
        Assert(bestBid->quantity == Quantity { 200'000'000 }, "invalid best bid quantity");
    }

    void testBestAsk()
    {
        OrderBook book;

        book.replace(
            100,
            {},
            {
                { Price { 6'500'002'000'000 }, Quantity { 100'000'000 } },
                { Price { 6'500'001'000'000 }, Quantity { 200'000'000 } },
                { Price { 6'500'003'000'000 }, Quantity { 300'000'000 } }
            });

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "best ask must exist");
        Assert(bestAsk->price == Price { 6'500'001'000'000 }, "invalid best ask");
        Assert(bestAsk->quantity == Quantity { 200'000'000 }, "invalid best ask quantity");
    }

    void testBestBidChangesAfterRemove()
    {
        OrderBook book;

        constexpr Price bestPrice { 6'500'000'000'000 };
        constexpr Price nextPrice { 6'499'999'000'000 };

        book.replace(
            100,
            {
                { bestPrice, Quantity { 100'000'000 } },
                { nextPrice, Quantity { 200'000'000 } }
            },
            {});

        Assert(
            book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 101,
                .side = Side::Buy,
                .price = bestPrice,
                .quantity = Quantity {}
            }),
            "best bid removal must be applied");

        const auto bestBid = book.bestBid();

        Assert(bestBid.has_value(), "next best bid must exist");
        Assert(bestBid->price == nextPrice, "next bid must become best bid");
        Assert(bestBid->quantity == Quantity { 200'000'000 }, "invalid next best bid quantity");
    }

    void testBestAskChangesAfterRemove()
    {
        OrderBook book;

        constexpr Price bestPrice { 6'500'001'000'000 };
        constexpr Price nextPrice { 6'500'002'000'000 };

        book.replace(
            100,
            {},
            {
                { bestPrice, Quantity { 100'000'000 } },
                { nextPrice, Quantity { 200'000'000 } }
            });

        Assert(
            book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 101,
                .side = Side::Sell,
                .price = bestPrice,
                .quantity = Quantity {}
            }),
            "best ask removal must be applied");

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "next best ask must exist");
        Assert(bestAsk->price == nextPrice, "next ask must become best ask");
        Assert(
            bestAsk->quantity == Quantity { 200'000'000 },
            "invalid next best ask quantity");
    }

    void testSequentialUpdates()
    {
        OrderBook book;
        book.replace(100, {}, {});

        Assert(book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 101,
                .side = Side::Buy,
                .price = Price { 6'500'001'000'000 },
                .quantity = Quantity { 100'000'000 }
            }), "sequence 101 must be accepted");

        Assert(book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 102,
                .side = Side::Buy,
                .price = Price { 6'500'002'000'000 },
                .quantity = Quantity { 200'000'000 }
            }),"sequence 102 must be accepted");

        Assert(book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 103,
                .side = Side::Sell,
                .price = Price { 6'500'003'000'000 },
                .quantity = Quantity { 300'000'000 }
            }),"sequence 103 must be accepted");

        Assert(book.sequence() == 103, "invalid final sequence");
        Assert(book.bidVolume(Price { 6'500'002'000'000 }) == Quantity { 200'000'000 }, "invalid final bid volume");
        Assert(book.askVolume(Price { 6'500'003'000'000 }) == Quantity { 300'000'000 }, "invalid final ask volume");
    }

    void testSequenceGap()
    {
        OrderBook book;
        constexpr Price price { 6'500'000'000'000 };

        book.replace(100, {}, {});
        Assert(book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 101,
                .side = Side::Buy,
                .price = price,
                .quantity = Quantity { 100'000'000 }
        }), "sequence 101 must be accepted");

        const bool applied = book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 103,
            .side = Side::Buy,
            .price = price,
            .quantity = Quantity { 200'000'000 }
        });

        Assert(!applied, "sequence gap must be rejected");
        Assert(book.sequence() == 101, "sequence must not advance after gap");
        Assert(book.bidVolume(price) == Quantity { 100'000'000 }, "book must not be modified after sequence gap");
    }

    void testStaleSequence()
    {
        OrderBook book;

        constexpr Price price { 6'500'000'000'000 };

        book.replace(100,
            {{ price, Quantity { 100'000'000 } }},
            {}
        );

        const bool applied = book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 100,
            .side = Side::Buy,
            .price = price,
            .quantity = Quantity { 200'000'000 }
        });

        Assert(!applied, "stale sequence must be rejected");
        Assert(book.sequence() == 100, "sequence must not change after stale update");
        Assert(book.bidVolume(price) == Quantity { 100'000'000 }, "book must not be modified after stale update");
    }

    void testRejectedUpdateDoesNotModifyBook()
    {
        OrderBook book;

        constexpr Price bidPrice { 6'500'000'000'000 };
        constexpr Price askPrice { 6'500'001'000'000 };

        book.replace(100,
            {{ bidPrice, Quantity { 100'000'000 } }},
            {{ askPrice, Quantity { 200'000'000 } }}
        );

        const bool applied = book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 102,
            .side = Side::Sell,
            .price = askPrice,
            .quantity = Quantity { 500'000'000 }
        });

        Assert(!applied, "sequence gap must be rejected");
        Assert(book.sequence() == 100, "sequence must not change after rejected update");
        Assert(book.bidVolume(bidPrice) == Quantity { 100'000'000 }, "bid side must not change after rejected update");
        Assert(book.askVolume(askPrice) == Quantity { 200'000'000 }, "ask side must not change after rejected update");
    }

    void testBidAndAskAreIndependent()
    {
        OrderBook book;

        constexpr Price bidPrice { 6'500'000'000'000 };
        constexpr Price askPrice { 6'500'001'000'000 };

        book.replace(100,
            { { bidPrice, Quantity { 100'000'000 } } },
            { { askPrice, Quantity { 200'000'000 } } }
        );

        Assert(book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 101,
                .side = Side::Buy,
                .price = bidPrice,
                .quantity = Quantity { 300'000'000 }
        }),"bid update must be applied");

        Assert(book.bidVolume(bidPrice) == Quantity { 300'000'000 }, "bid must be updated");
        Assert(book.askVolume(askPrice) == Quantity { 200'000'000 }, "ask must not be modified by bid update");
        Assert(book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 102,
                .side = Side::Sell,
                .price = askPrice,
                .quantity = Quantity { 400'000'000 }
        }),"ask update must be applied");

        Assert(book.bidVolume(bidPrice) == Quantity { 300'000'000 }, "bid must not be modified by ask update");
        Assert(book.askVolume(askPrice) == Quantity { 400'000'000 }, "ask must be updated");
    }

    void testReplaceClearsPreviousLevels()
    {
        OrderBook book;

        book.replace(100,
            { { Price { 6'500'000'000'000 }, Quantity { 100'000'000 } } },
            { { Price { 6'500'001'000'000 }, Quantity { 200'000'000 } } }
        );
        book.replace(200,
            { { Price { 6'600'000'000'000 }, Quantity { 300'000'000 } } },
            { { Price { 6'600'001'000'000 }, Quantity { 400'000'000 } } }
        );

        Assert(book.sequence() == 200, "replace sequence must replace old sequence");
        Assert(book.bidVolume(Price { 6'500'000'000'000 }).isZero(), "old bid must be removed");
        Assert(book.askVolume(Price { 6'500'001'000'000 }).isZero(), "old ask must be removed");
        Assert(book.bidVolume(Price { 6'600'000'000'000 }) == Quantity { 300'000'000 }, "new bid must exist");
        Assert(book.askVolume(Price { 6'600'001'000'000 }) == Quantity { 400'000'000 }, "new ask must exist");
    }

    void testReplaceWithEmptySide()
    {
        OrderBook book;

        constexpr Price bidPrice { 6'500'000'000'000 };
        constexpr Price askPrice { 6'500'001'000'000 };

        book.replace(100,
            { { bidPrice, Quantity { 100'000'000 } } },
            { { askPrice, Quantity { 200'000'000 } } }
        );
        book.replace(200,
            {},
            { { Price { 6'600'001'000'000 }, Quantity { 300'000'000 } } }
        );

        Assert(book.sequence() == 200, "replace sequence must be updated");
        Assert(!book.bestBid().has_value(), "bids must be cleared by empty snapshot side");
        Assert(book.bidVolume(bidPrice).isZero(), "old bid must be removed by empty snapshot side");

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "new ask must exist");
        Assert(bestAsk->price == Price { 6'600'001'000'000 },"invalid new best ask");
        Assert(bestAsk->quantity == Quantity { 300'000'000 },"invalid new best ask quantity");
        Assert(book.askVolume(askPrice).isZero(),"old ask must be removed");
    }

    void testClear()
    {
        OrderBook book;
        book.replace(100,
            { { Price { 6'500'000'000'000 }, Quantity { 100'000'000 } } },
            { { Price { 6'500'001'000'000 }, Quantity { 200'000'000 } } }
        );

        book.clear();
        Assert(book.sequence() == 0, "cleared book sequence must be zero");
        Assert(!book.bestBid().has_value(), "cleared book must have no bid");
        Assert(!book.bestAsk().has_value(), "cleared book must have no ask");
    }

    void testClearAfterUpdates()
    {
        OrderBook book;

        constexpr Price bidPrice { 6'500'000'000'000 };
        constexpr Price askPrice { 6'500'001'000'000 };

        book.replace(100, {}, {});

        Assert(book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 101,
                .side = Side::Buy,
                .price = bidPrice,
                .quantity = Quantity { 100'000'000 }
        }),"bid update must be applied");

        Assert(book.applyUpdate(BookUpdate {
                .instrument = 1,
                .sequence = 102,
                .side = Side::Sell,
                .price = askPrice,
                .quantity = Quantity { 200'000'000 }
        }),"ask update must be applied");

        book.clear();

        Assert(book.sequence() == 0, "cleared book sequence must be zero");
        Assert(book.bidVolume(bidPrice).isZero(), "cleared bid volume must be zero");
        Assert(book.askVolume(askPrice).isZero(), "cleared ask volume must be zero");
        Assert(!book.bestBid().has_value(), "cleared book must have no bid");
        Assert(!book.bestAsk().has_value(), "cleared book must have no ask");
    }
}

void order_book_test()
{
    testEmptyBook();
    testReplace();
    testAddBid();
    testAddAsk();
    testUpdateExistingLevel();
    testRemoveLevel();
    testRemoveUnknownLevel();
    testBestBid();
    testBestAsk();
    testBestBidChangesAfterRemove();
    testBestAskChangesAfterRemove();
    testSequentialUpdates();
    testSequenceGap();
    testStaleSequence();
    testRejectedUpdateDoesNotModifyBook();
    testBidAndAskAreIndependent();
    testReplaceClearsPreviousLevels();
    testReplaceWithEmptySide();
    testClear();
    testClearAfterUpdates();

    std::cout << "All OrderBook tests: OK\n";
}