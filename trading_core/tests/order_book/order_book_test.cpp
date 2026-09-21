/**============================================================================
Name        : order_book_test.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : OrderBook tests.
============================================================================**/

#include "order_book/order_book.hpp"
#include "test_support/testing.hpp"

#include <iostream>

namespace
{
    using trading::Price;
    using trading::Quantity;
    using trading::SequenceNumber;
    using trading::Side;
    using trading::market_data::BookUpdate;
    using trading::order_book::OrderBook;
    using testing::Assert;

    constexpr Price BestBidPrice { 6'500'000'000'000 };
    constexpr Price SecondBidPrice { 6'499'999'000'000 };
    constexpr Price ThirdBidPrice { 6'499'998'000'000 };

    constexpr Price BestAskPrice { 6'500'001'000'000 };
    constexpr Price SecondAskPrice { 6'500'002'000'000 };
    constexpr Price ThirdAskPrice { 6'500'003'000'000 };

    constexpr Quantity FirstQuantity { 100'000'000 };
    constexpr Quantity SecondQuantity { 200'000'000 };
    constexpr Quantity ThirdQuantity { 300'000'000 };


    BookUpdate createBidUpdate(const SequenceNumber sequence,
                               const Price price,
                               const Quantity quantity)
    {
        return BookUpdate {
            .instrument = 1,
            .sequence = sequence,
            .side = Side::Buy,
            .price = price,
            .quantity = quantity
        };
    }


    BookUpdate createAskUpdate(const SequenceNumber sequence,
                               const Price price,
                               const Quantity quantity)
    {
        return BookUpdate {
            .instrument = 1,
            .sequence = sequence,
            .side = Side::Sell,
            .price = price,
            .quantity = quantity
        };
    }


    void testEmptyBook()
    {
        const OrderBook book;

        Assert(book.sequence() == 0, "new book sequence must be zero");
        Assert(book.bidSize() == 0, "new book must have no bids");
        Assert(book.askSize() == 0, "new book must have no asks");
        Assert(!book.bestBid().has_value(), "empty book must have no best bid");
        Assert(!book.bestAsk().has_value(), "empty book must have no best ask");
    }

    void testSetState()
    {
        OrderBook book;

        const OrderBook::Levels bids {
            { BestBidPrice, FirstQuantity },
            { SecondBidPrice, SecondQuantity }
        };

        const OrderBook::Levels asks {
            { BestAskPrice, FirstQuantity },
            { SecondAskPrice, ThirdQuantity }
        };

        book.setState(100, bids, asks);

        Assert(book.sequence() == 100, "setState sequence must be stored");
        Assert(book.bidSize() == 2, "setState must store all bid levels");
        Assert(book.askSize() == 2, "setState must store all ask levels");

        const auto bestBid = book.bestBid();

        Assert(bestBid.has_value(), "best bid must exist");
        Assert(bestBid->price == BestBidPrice, "invalid best bid price");
        Assert(bestBid->quantity == FirstQuantity, "invalid best bid quantity");

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "best ask must exist");
        Assert(bestAsk->price == BestAskPrice, "invalid best ask price");
        Assert(bestAsk->quantity == FirstQuantity, "invalid best ask quantity");
    }

    void testSetStateClearsPreviousLevels()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },
            {
                { BestAskPrice, SecondQuantity }
            });

        book.setState(200,
            {
                { ThirdBidPrice, ThirdQuantity }
            },
            {
                { ThirdAskPrice, FirstQuantity }
            });

        Assert(book.sequence() == 200, "setState sequence must replace old sequence");
        Assert(book.bidVolume(BestBidPrice).isZero(), "old bid must be removed");
        Assert(book.askVolume(BestAskPrice).isZero(), "old ask must be removed");
        Assert(book.bidVolume(ThirdBidPrice) == ThirdQuantity, "new bid must exist");
        Assert(book.askVolume(ThirdAskPrice) == FirstQuantity, "new ask must exist");
    }


    void testSetStateWithEmptySide()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },
            {
                { BestAskPrice, SecondQuantity }
            });

        book.setState(200,
            {},
            {
                { SecondAskPrice, ThirdQuantity }
            });

        Assert(book.sequence() == 200, "setState sequence must be updated");
        Assert(book.bidSize() == 0, "empty bid side must clear previous bids");
        Assert(book.askSize() == 1, "ask side must contain new level");
        Assert(!book.bestBid().has_value(), "empty bid side must have no best bid");
        Assert(book.bidVolume(BestBidPrice).isZero(), "old bid must be removed");

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "new ask must exist");
        Assert(bestAsk->price == SecondAskPrice, "invalid new best ask");
        Assert(bestAsk->quantity == ThirdQuantity, "invalid new best ask quantity");
        Assert(book.askVolume(BestAskPrice).isZero(), "old ask must be removed");
    }


    void testAddBid()
    {
        OrderBook book;

        book.setState(100, {}, {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, FirstQuantity)),
            "bid update must be applied");

        Assert(book.sequence() == 101, "sequence must be updated");
        Assert(book.bidSize() == 1, "bid level must be added");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "invalid bid quantity");

        const auto bestBid = book.bestBid();

        Assert(bestBid.has_value(), "best bid must exist");
        Assert(bestBid->price == BestBidPrice, "invalid bid price");
        Assert(bestBid->quantity == FirstQuantity, "invalid bid quantity");
    }


    void testAddAsk()
    {
        OrderBook book;

        book.setState(100, {}, {});

        Assert(book.applyUpdate(createAskUpdate(101, BestAskPrice, FirstQuantity)),
            "ask update must be applied");

        Assert(book.sequence() == 101, "sequence must be updated");
        Assert(book.askSize() == 1, "ask level must be added");
        Assert(book.askVolume(BestAskPrice) == FirstQuantity, "invalid ask quantity");

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "best ask must exist");
        Assert(bestAsk->price == BestAskPrice, "invalid ask price");
        Assert(bestAsk->quantity == FirstQuantity, "invalid ask quantity");
    }


    void testUpdateExistingBidLevel()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },
            {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, ThirdQuantity)),
            "bid level update must be applied");

        Assert(book.sequence() == 101, "sequence must be updated");
        Assert(book.bidSize() == 1, "existing bid update must not add a level");
        Assert(book.bidVolume(BestBidPrice) == ThirdQuantity, "bid quantity must be updated");
    }


    void testUpdateExistingAskLevel()
    {
        OrderBook book;

        book.setState(100,
            {},
            {
                { BestAskPrice, FirstQuantity }
            });

        Assert(book.applyUpdate(createAskUpdate(101, BestAskPrice, ThirdQuantity)),
            "ask level update must be applied");

        Assert(book.sequence() == 101, "sequence must be updated");
        Assert(book.askSize() == 1, "existing ask update must not add a level");
        Assert(book.askVolume(BestAskPrice) == ThirdQuantity, "ask quantity must be updated");
    }


    void testRemoveBidLevel()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },
            {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, {})),
            "bid removal must be applied");

        Assert(book.sequence() == 101, "sequence must be updated after removal");
        Assert(book.bidSize() == 0, "removed bid must no longer exist");
        Assert(book.bidVolume(BestBidPrice).isZero(), "removed bid must have zero volume");
        Assert(!book.bestBid().has_value(), "book must have no best bid");
    }


    void testRemoveAskLevel()
    {
        OrderBook book;

        book.setState(100,
            {},
            {
                { BestAskPrice, FirstQuantity }
            });

        Assert(book.applyUpdate(createAskUpdate(101, BestAskPrice, {})),
            "ask removal must be applied");

        Assert(book.sequence() == 101, "sequence must be updated after removal");
        Assert(book.askSize() == 0, "removed ask must no longer exist");
        Assert(book.askVolume(BestAskPrice).isZero(), "removed ask must have zero volume");
        Assert(!book.bestAsk().has_value(), "book must have no best ask");
    }


    void testRemoveUnknownBidLevel()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },
            {});

        Assert(!book.applyUpdate(createBidUpdate(101, SecondBidPrice, {})),
            "remove of unknown bid level must be rejected");

        Assert(book.sequence() == 100, "sequence must not change after rejected removal");
        Assert(book.bidSize() == 1, "existing bid must remain");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "existing bid must not change");
        Assert(book.bidVolume(SecondBidPrice).isZero(), "unknown bid must remain absent");
    }


    void testRemoveUnknownAskLevel()
    {
        OrderBook book;

        book.setState(100,
            {},
            {
                { BestAskPrice, FirstQuantity }
            });

        Assert(!book.applyUpdate(createAskUpdate(101, SecondAskPrice, {})),
            "remove of unknown ask level must be rejected");

        Assert(book.sequence() == 100, "sequence must not change after rejected removal");
        Assert(book.askSize() == 1, "existing ask must remain");
        Assert(book.askVolume(BestAskPrice) == FirstQuantity, "existing ask must not change");
        Assert(book.askVolume(SecondAskPrice).isZero(), "unknown ask must remain absent");
    }


    void testBestBid()
    {
        OrderBook book;

        book.setState(100,
            {
                { SecondBidPrice, FirstQuantity },
                { BestBidPrice, SecondQuantity },
                { ThirdBidPrice, ThirdQuantity }
            },
            {});

        const auto bestBid = book.bestBid();

        Assert(bestBid.has_value(), "best bid must exist");
        Assert(bestBid->price == BestBidPrice, "invalid best bid price");
        Assert(bestBid->quantity == SecondQuantity, "invalid best bid quantity");
    }


    void testBestAsk()
    {
        OrderBook book;

        book.setState(100,
            {},
            {
                { SecondAskPrice, FirstQuantity },
                { BestAskPrice, SecondQuantity },
                { ThirdAskPrice, ThirdQuantity }
            });

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "best ask must exist");
        Assert(bestAsk->price == BestAskPrice, "invalid best ask price");
        Assert(bestAsk->quantity == SecondQuantity, "invalid best ask quantity");
    }


    void testBestBidChangesAfterRemove()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity },
                { SecondBidPrice, SecondQuantity }
            },
            {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, {})),
            "best bid removal must be applied");

        const auto bestBid = book.bestBid();

        Assert(bestBid.has_value(), "next best bid must exist");
        Assert(bestBid->price == SecondBidPrice, "next bid must become best bid");
        Assert(bestBid->quantity == SecondQuantity, "invalid next best bid quantity");
    }


    void testBestAskChangesAfterRemove()
    {
        OrderBook book;

        book.setState(100,
            {},
            {
                { BestAskPrice, FirstQuantity },
                { SecondAskPrice, SecondQuantity }
            });

        Assert(book.applyUpdate(createAskUpdate(101, BestAskPrice, {})),
            "best ask removal must be applied");

        const auto bestAsk = book.bestAsk();

        Assert(bestAsk.has_value(), "next best ask must exist");
        Assert(bestAsk->price == SecondAskPrice, "next ask must become best ask");
        Assert(bestAsk->quantity == SecondQuantity, "invalid next best ask quantity");
    }


    void testSequentialUpdates()
    {
        OrderBook book;
        book.setState(100, {}, {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, FirstQuantity)),
            "sequence 101 must be accepted");
        Assert(book.applyUpdate(createBidUpdate(102, SecondBidPrice, SecondQuantity)),
            "sequence 102 must be accepted");
        Assert(book.applyUpdate(createAskUpdate(103, BestAskPrice, ThirdQuantity)),
            "sequence 103 must be accepted");
        Assert(book.sequence() == 103, "invalid final sequence");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "invalid first bid volume");
        Assert(book.bidVolume(SecondBidPrice) == SecondQuantity, "invalid second bid volume");
        Assert(book.askVolume(BestAskPrice) == ThirdQuantity, "invalid ask volume");
    }


    void testSequenceGap()
    {
        OrderBook book;

        book.setState(100, {}, {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, FirstQuantity)),
            "sequence 101 must be accepted");
        Assert(!book.applyUpdate(createBidUpdate(103, BestBidPrice, SecondQuantity)),
            "sequence gap must be rejected");
        Assert(book.sequence() == 101, "sequence must not advance after gap");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "book must not change after sequence gap");
    }


    void testStaleSequence()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },{});

        Assert(!book.applyUpdate(createBidUpdate(100, BestBidPrice, SecondQuantity)),
            "stale sequence must be rejected");
        Assert(book.sequence() == 100, "sequence must not change after stale update");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "book must not change after stale update");
    }


    void testRejectedUpdateDoesNotModifyBook()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },
            {
                { BestAskPrice, SecondQuantity }
            });

        Assert(!book.applyUpdate(createAskUpdate(102, BestAskPrice, ThirdQuantity)),
            "sequence gap must be rejected");
        Assert(book.sequence() == 100, "sequence must not change after rejected update");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "bid must not change after rejected update");
        Assert(book.askVolume(BestAskPrice) == SecondQuantity, "ask must not change after rejected update");
    }


    void testBidAndAskAreIndependent()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },
            {
                { BestAskPrice, SecondQuantity }
            });

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, ThirdQuantity)),
            "bid update must be applied");
        Assert(book.bidVolume(BestBidPrice) == ThirdQuantity, "bid must be updated");
        Assert(book.askVolume(BestAskPrice) == SecondQuantity, "ask must not be modified by bid update");
        Assert(book.applyUpdate(createAskUpdate(102, BestAskPrice, FirstQuantity)),
            "ask update must be applied");
        Assert(book.bidVolume(BestBidPrice) == ThirdQuantity, "bid must not be modified by ask update");
        Assert(book.askVolume(BestAskPrice) == FirstQuantity, "ask must be updated");
    }


    void testBidDepthLimit()
    {
        constexpr OrderBook::size_type Depth { 2 };

        OrderBook book { Depth };
        book.setState(100, {}, {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, FirstQuantity)),
            "first bid inside depth must be accepted");

        Assert(book.applyUpdate(createBidUpdate(102, SecondBidPrice, SecondQuantity)),
            "second bid inside depth must be accepted");

        Assert(book.bidSize() == Depth, "bid side must not exceed depth");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "first bid must exist");
        Assert(book.bidVolume(SecondBidPrice) == SecondQuantity, "second bid must exist");
    }


    void testAskDepthLimit()
    {
        constexpr OrderBook::size_type Depth { 2 };

        OrderBook book { Depth };
        book.setState(100, {}, {});

        Assert(book.applyUpdate(createAskUpdate(101, BestAskPrice, FirstQuantity)),
            "first ask inside depth must be accepted");
        Assert(book.applyUpdate(createAskUpdate(102, SecondAskPrice, SecondQuantity)),
            "second ask inside depth must be accepted");

        Assert(book.askSize() == Depth, "ask side must not exceed depth");
        Assert(book.askVolume(BestAskPrice) == FirstQuantity, "first ask must exist");
        Assert(book.askVolume(SecondAskPrice) == SecondQuantity, "second ask must exist");
    }


    void testBidUpdateAtDepthDoesNotIncreaseSize()
    {
        constexpr OrderBook::size_type Depth { 2 };
        constexpr Price BetterBidPrice { 6'500'000'500'000 };

        OrderBook book { Depth };
        book.setState(100,
            {
                { BestBidPrice, FirstQuantity },
                { SecondBidPrice, SecondQuantity }
            },
            {});

        Assert(book.applyUpdate(createBidUpdate(101, BetterBidPrice, ThirdQuantity)),
            "better bid outside current depth must replace worst bid");

        Assert(book.bidSize() == Depth, "bid size must remain equal to depth");
        Assert(book.bidVolume(BetterBidPrice) == ThirdQuantity, "new better bid must be inserted");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "best bid must remain");
        Assert(book.bidVolume(SecondBidPrice).isZero(), "worst bid must be removed");
    }

    void testAskUpdateAtDepthDoesNotIncreaseSize()
    {
        constexpr OrderBook::size_type Depth { 2 };
        constexpr Price BetterAskPrice { 6'500'000'500'000 };

        OrderBook book { Depth };
        book.setState(100,
            {},
            {
                { BestAskPrice, FirstQuantity },
                { SecondAskPrice, SecondQuantity }
            });

        Assert(book.applyUpdate(createAskUpdate(101, BetterAskPrice, ThirdQuantity)),
            "better ask outside current depth must replace worst ask");

        Assert(book.askSize() == Depth, "ask size must remain equal to depth");
        Assert(book.askVolume(BetterAskPrice) == ThirdQuantity, "new better ask must be inserted");
        Assert(book.askVolume(BestAskPrice) == FirstQuantity, "best ask must remain");
        Assert(book.askVolume(SecondAskPrice).isZero(), "worst ask must be removed");
    }

    void testBidUpdateWorseThanDepthIsRejected()
    {
        constexpr OrderBook::size_type Depth { 2 };
        constexpr Price WorseBidPrice { 6'499'998'000'000 };

        OrderBook book { Depth };
        book.setState(100,
            {
                { BestBidPrice, FirstQuantity },
                { SecondBidPrice, SecondQuantity }
            },
            {});

        Assert(!book.applyUpdate(createBidUpdate(101, WorseBidPrice, ThirdQuantity)),
            "bid worse than depth must be rejected");

        Assert(book.sequence() == 100, "sequence must not advance after depth rejection");
        Assert(book.bidSize() == Depth, "bid size must remain unchanged");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "best bid must remain unchanged");
        Assert(book.bidVolume(SecondBidPrice) == SecondQuantity, "worst bid must remain unchanged");
        Assert(book.bidVolume(WorseBidPrice).isZero(), "worse bid must not be inserted");
    }

    void testAskUpdateWorseThanDepthIsRejected()
    {
        constexpr OrderBook::size_type Depth { 2 };
        constexpr Price WorseAskPrice { 6'500'003'000'000 };

        OrderBook book { Depth };
        book.setState(100,
            {},
            {
                { BestAskPrice, FirstQuantity },
                { SecondAskPrice, SecondQuantity }
            });

        Assert(!book.applyUpdate(createAskUpdate(101, WorseAskPrice, ThirdQuantity)),
            "ask worse than depth must be rejected");

        Assert(book.sequence() == 100, "sequence must not advance after depth rejection");
        Assert(book.askSize() == Depth, "ask size must remain unchanged");
        Assert(book.askVolume(BestAskPrice) == FirstQuantity, "best ask must remain unchanged");
        Assert(book.askVolume(SecondAskPrice) == SecondQuantity, "worst ask must remain unchanged");
        Assert(book.askVolume(WorseAskPrice).isZero(), "worse ask must not be inserted");
    }

    void testBidUpdateAtDepthIsAcceptedWhenItImprovesPrice()
    {
        constexpr OrderBook::size_type Depth { 3 };

        OrderBook book { Depth };
        book.setState(100,
            {
                { BestBidPrice, FirstQuantity },
                { SecondBidPrice, SecondQuantity },
                { ThirdBidPrice, ThirdQuantity }
            },
            {});

        constexpr Price BetterBidPrice { 6'500'000'500'000 };

        Assert(book.applyUpdate(createBidUpdate(101, BetterBidPrice, FirstQuantity)),
            "better bid must be accepted when depth is full");

        Assert(book.sequence() == 101, "sequence must advance after accepted update");
        Assert(book.bidSize() == Depth, "bid size must remain equal to depth");
        Assert(book.bidVolume(BetterBidPrice) == FirstQuantity, "better bid must exist");
        Assert(book.bidVolume(ThirdBidPrice).isZero(), "old worst bid must be removed");
        Assert(book.bestBid()->price == BetterBidPrice, "better bid must become best bid");
    }


    void testAskUpdateAtDepthIsAcceptedWhenItImprovesPrice()
    {
        constexpr OrderBook::size_type Depth { 3 };

        OrderBook book { Depth };
        book.setState(100,
            {},
            {
                { BestAskPrice, FirstQuantity },
                { SecondAskPrice, SecondQuantity },
                { ThirdAskPrice, ThirdQuantity }
            });

        constexpr Price BetterAskPrice { 6'500'000'500'000 };

        Assert(book.applyUpdate(createAskUpdate(101, BetterAskPrice, FirstQuantity)),
            "better ask must be accepted when depth is full");

        Assert(book.sequence() == 101, "sequence must advance after accepted update");
        Assert(book.askSize() == Depth, "ask size must remain equal to depth");
        Assert(book.askVolume(BetterAskPrice) == FirstQuantity, "better ask must exist");
        Assert(book.askVolume(ThirdAskPrice).isZero(), "old worst ask must be removed");
        Assert(book.bestAsk()->price == BetterAskPrice, "better ask must become best ask");
    }

    void testUpdateExistingLevelAtFullDepthIsAccepted()
    {
        constexpr OrderBook::size_type Depth { 2 };

        OrderBook book { Depth };
        book.setState(100,
            {
                { BestBidPrice, FirstQuantity },
                { SecondBidPrice, SecondQuantity }
            },
            {
                { BestAskPrice, FirstQuantity },
                { SecondAskPrice, SecondQuantity }
            });

        Assert(book.applyUpdate(createBidUpdate(101, SecondBidPrice, ThirdQuantity)),
            "existing bid update must be accepted at full depth");
        Assert(book.applyUpdate(createAskUpdate(102, SecondAskPrice, ThirdQuantity)),
            "existing ask update must be accepted at full depth");
        Assert(book.bidSize() == Depth, "bid size must remain unchanged");
        Assert(book.askSize() == Depth, "ask size must remain unchanged");
        Assert(book.bidVolume(SecondBidPrice) == ThirdQuantity, "existing bid must be updated");
        Assert(book.askVolume(SecondAskPrice) == ThirdQuantity, "existing ask must be updated");
    }

    void testRemoveAtFullDepth()
    {
        constexpr OrderBook::size_type Depth { 2 };

        OrderBook book { Depth };
        book.setState(100,
            {
                { BestBidPrice, FirstQuantity },
                { SecondBidPrice, SecondQuantity }
            },
            {
                { BestAskPrice, FirstQuantity },
                { SecondAskPrice, SecondQuantity }
            });

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, {})),
            "bid removal must be accepted at full depth");
        Assert(book.applyUpdate(createAskUpdate(102, BestAskPrice, {})),
            "ask removal must be accepted at full depth");
        Assert(book.bidSize() == 1, "bid size must decrease after removal");
        Assert(book.askSize() == 1, "ask size must decrease after removal");
        Assert(book.bestBid()->price == SecondBidPrice, "second bid must become best");
        Assert(book.bestAsk()->price == SecondAskPrice, "second ask must become best");
    }

    void testDepthLimitIsIndependentForBidAndAsk()
    {
        constexpr OrderBook::size_type Depth { 2 };

        OrderBook book { Depth };
        book.setState(100, {}, {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, FirstQuantity)),
            "bid update must be accepted");
        Assert(book.applyUpdate(createBidUpdate(102, SecondBidPrice, SecondQuantity)),
            "second bid update must be accepted");
        Assert(book.applyUpdate(createAskUpdate(103, BestAskPrice, FirstQuantity)),
            "ask update must be accepted");
        Assert(book.applyUpdate(createAskUpdate(104, SecondAskPrice, SecondQuantity)),
            "second ask update must be accepted");
        Assert(book.bidSize() == Depth, "bid depth must be independent");
        Assert(book.askSize() == Depth, "ask depth must be independent");
        Assert(book.bidVolume(BestBidPrice) == FirstQuantity, "bid levels must be preserved");
        Assert(book.askVolume(BestAskPrice) == FirstQuantity, "ask levels must be preserved");
    }

    void testClear()
    {
        OrderBook book;

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity }
            },
            {
                { BestAskPrice, SecondQuantity }
            });

        book.clear();

        Assert(book.sequence() == 0, "cleared book sequence must be zero");
        Assert(book.bidSize() == 0, "cleared book must have no bids");
        Assert(book.askSize() == 0, "cleared book must have no asks");
        Assert(!book.bestBid().has_value(), "cleared book must have no bid");
        Assert(!book.bestAsk().has_value(), "cleared book must have no ask");
    }

    void testClearAfterUpdates()
    {
        OrderBook book;

        book.setState(100, {}, {});

        Assert(book.applyUpdate(createBidUpdate(101, BestBidPrice, FirstQuantity)),
            "bid update must be applied");
        Assert(book.applyUpdate(createAskUpdate(102, BestAskPrice, SecondQuantity)),
            "ask update must be applied");

        book.clear();

        Assert(book.sequence() == 0, "cleared book sequence must be zero");
        Assert(book.bidSize() == 0, "cleared bid side must be empty");
        Assert(book.askSize() == 0, "cleared ask side must be empty");
        Assert(book.bidVolume(BestBidPrice).isZero(), "cleared bid volume must be zero");
        Assert(book.askVolume(BestAskPrice).isZero(), "cleared ask volume must be zero");
        Assert(!book.bestBid().has_value(), "cleared book must have no best bid");
        Assert(!book.bestAsk().has_value(), "cleared book must have no best ask");
    }


    void testDepthIsPreservedAfterClear()
    {
        constexpr OrderBook::size_type Depth { 2 };

        OrderBook book { Depth };

        book.setState(100,
            {
                { BestBidPrice, FirstQuantity },
                { SecondBidPrice, SecondQuantity }
            },
            {});

        book.clear();

        Assert(book.applyUpdate(createBidUpdate(1, BestBidPrice, FirstQuantity)),
            "book must remain usable after clear");
        Assert(book.applyUpdate(createBidUpdate(2, SecondBidPrice, SecondQuantity)),
            "second level must be accepted after clear");

        constexpr Price WorseBidPrice { 6'499'998'000'000 };

        Assert(!book.applyUpdate(createBidUpdate(3, WorseBidPrice, ThirdQuantity)),
            "depth limit must remain after clear");
        Assert(book.bidSize() == Depth, "depth must remain unchanged after clear");
    }
}


void order_book_test()
{
    testEmptyBook();

    testSetState();
    testSetStateClearsPreviousLevels();
    testSetStateWithEmptySide();

    testAddBid();
    testAddAsk();
    testUpdateExistingBidLevel();
    testUpdateExistingAskLevel();
    testRemoveBidLevel();
    testRemoveAskLevel();
    testRemoveUnknownBidLevel();
    testRemoveUnknownAskLevel();

    testBestBid();
    testBestAsk();
    testBestBidChangesAfterRemove();
    testBestAskChangesAfterRemove();

    testSequentialUpdates();
    testSequenceGap();
    testStaleSequence();
    testRejectedUpdateDoesNotModifyBook();
    testBidAndAskAreIndependent();

    testBidDepthLimit();
    testAskDepthLimit();
    testBidUpdateAtDepthDoesNotIncreaseSize();
    testAskUpdateAtDepthDoesNotIncreaseSize();
    testBidUpdateWorseThanDepthIsRejected();
    testAskUpdateWorseThanDepthIsRejected();
    testBidUpdateAtDepthIsAcceptedWhenItImprovesPrice();
    testAskUpdateAtDepthIsAcceptedWhenItImprovesPrice();
    testUpdateExistingLevelAtFullDepthIsAccepted();
    testRemoveAtFullDepth();
    testDepthLimitIsIndependentForBidAndAsk();

    testClear();
    testClearAfterUpdates();
    testDepthIsPreservedAfterClear();


    std::cout << "All OrderBook tests: OK\n";
}
