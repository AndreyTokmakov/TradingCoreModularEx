#include "price.hpp"
#include "test_support/testing.hpp"

#include <cstdint>
#include <iostream>
#include <limits>

namespace
{
    using trading::Price;
    using testing::Assert;


    void testDefaultConstruction()
    {
        constexpr Price price;

        Assert(price.raw() == 0, "default price raw value must be zero");
        Assert(price.isZero(), "default price must be zero");
        Assert(!price.isPositive(), "default price must not be positive");
    }

    void testRawConstruction()
    {
        constexpr Price price { 6'500'000'000'000 };
        Assert(price.raw() == 6'500'000'000'000,"price must preserve raw constructor value");
    }

    void testFromInteger()
    {
        constexpr auto price = Price::fromInteger(65'000);
        Assert(price.raw() == 65'000 * Price::Scale,"fromInteger must multiply value by scale");
    }

    void testScale()
    {
        Assert(Price::DecimalPlaces == 8, "price must have eight decimal places");
        Assert(Price::Scale == 100'000'000, "price scale must be 100000000");
    }

    void testZeroPrice()
    {
        constexpr Price price {};

        Assert(price.isZero(), "zero price must report isZero");
        Assert(!price.isPositive(), "zero price must not be positive");
        Assert(price.raw() == 0, "zero price raw value must be zero");
    }

    void testPositivePrice()
    {
        constexpr Price price { 1 };

        Assert(!price.isZero(), "positive price must not be zero");
        Assert(price.isPositive(), "positive price must report isPositive");
    }

    void testNegativePrice()
    {
        constexpr Price price { -1 };

        Assert(!price.isZero(), "negative price must not be zero");
        Assert(!price.isPositive(), "negative price must not be positive");
        Assert(price.raw() == -1, "negative raw price must be preserved");
    }

    void testAddition()
    {
        constexpr Price first { 6'500'000'000'000 };
        constexpr Price second { 250'000'000 };

        constexpr Price result = first + second;

        Assert(result == Price { 6'500'250'000'000 },"price addition must add raw values");
        Assert(first == Price { 6'500'000'000'000 },"addition must not modify left operand");
        Assert(second == Price { 250'000'000 },"addition must not modify right operand");
    }

    void testAdditionWithZero()
    {
        constexpr Price price { 6'500'000'000'000 };
        constexpr Price zero {};

        Assert(price + zero == price, "adding zero must not change price");
        Assert(zero + price == price, "zero plus price must equal price");
    }

    void testSubtraction()
    {
        constexpr Price first { 6'500'000'000'000 };
        constexpr Price second { 250'000'000 };

        constexpr Price result = first - second;

        Assert(result == Price { 6'499'750'000'000 },"price subtraction must subtract raw values");
        Assert(first == Price { 6'500'000'000'000 },"subtraction must not modify left operand");
        Assert(second == Price { 250'000'000 },"subtraction must not modify right operand");
    }

    void testSubtractionWithZero()
    {
        constexpr Price price { 6'500'000'000'000 };
        constexpr Price zero {};

        Assert(price - zero == price, "subtracting zero must not change price");
        Assert(zero - price == Price { -6'500'000'000'000 }, "zero minus price must negate price");
    }

    void testMultiplication()
    {
        constexpr Price price { 6'500'000'000'000 };

        constexpr Price result = price * 3;

        Assert(result == Price { 19'500'000'000'000 },"price multiplication must multiply raw value");
        Assert(price == Price { 6'500'000'000'000 },"multiplication must not modify original price");
    }

    void testMultiplicationByZero()
    {
        constexpr Price price { 6'500'000'000'000 };
        Assert(price * 0 == Price {}, "multiplication by zero must produce zero price");
    }

    void testMultiplicationByNegativeValue()
    {
        constexpr Price price { 6'500'000'000'000 };
        Assert(price * -1 == Price { -6'500'000'000'000 },"multiplication by negative value must negate price");
    }

    void testAdditionAssignment()
    {
        Price price { 6'500'000'000'000 };
        price += Price { 250'000'000 };

        Assert(price == Price { 6'500'250'000'000 },"operator+= must update price");
    }

    void testAdditionAssignmentWithZero()
    {
        Price price { 6'500'000'000'000 };
        price += Price {};

        Assert(price == Price { 6'500'000'000'000 },"operator+= with zero must not change price");
    }

    void testSubtractionAssignment()
    {
        Price price { 6'500'000'000'000 };
        price -= Price { 250'000'000 };

        Assert(price == Price { 6'499'750'000'000 },"operator-= must update price");
    }

    void testSubtractionAssignmentWithZero()
    {
        Price price { 6'500'000'000'000 };

        price -= Price {};
        Assert(price == Price { 6'500'000'000'000 },"operator-= with zero must not change price");
    }

    void testEquality()
    {
        constexpr Price first { 6'500'000'000'000 };
        constexpr Price second { 6'500'000'000'000 };
        constexpr Price different { 6'500'000'000'001 };

        Assert(first == second, "prices with equal raw values must be equal");
        Assert(first != different, "prices with different raw values must not be equal");
    }

    void testLessComparison()
    {
        constexpr Price lower { 6'499'999'999'999 };
        constexpr Price higher { 6'500'000'000'000 };

        Assert(lower < higher, "lower price must compare less than higher price");
        Assert(!(higher < lower), "higher price must not compare less than lower price");
    }

    void testLessOrEqualComparison()
    {
        constexpr Price first { 6'500'000'000'000 };
        constexpr Price second { 6'500'000'000'000 };
        constexpr Price higher { 6'500'000'000'001 };

        Assert(first <= second, "equal prices must compare less or equal");
        Assert(first <= higher, "lower price must compare less or equal");
        Assert(!(higher <= first), "higher price must not compare less or equal");
    }

    void testGreaterComparison()
    {
        constexpr Price lower { 6'499'999'999'999 };
        constexpr Price higher { 6'500'000'000'000 };

        Assert(higher > lower, "higher price must compare greater than lower price");
        Assert(!(lower > higher), "lower price must not compare greater than higher price");
    }

    void testGreaterOrEqualComparison()
    {
        constexpr Price first { 6'500'000'000'000 };
        constexpr Price second { 6'500'000'000'000 };
        constexpr Price lower { 6'499'999'999'999 };

        Assert(first >= second, "equal prices must compare greater or equal");
        Assert(first >= lower, "higher price must compare greater or equal");
        Assert(!(lower >= first), "lower price must not compare greater or equal");
    }

    void testOrdering()
    {
        constexpr Price negative { -1 };
        constexpr Price zero {};
        constexpr Price positive { 1 };

        Assert(negative < zero, "negative price must be less than zero");
        Assert(zero < positive, "zero price must be less than positive");
        Assert(negative < positive, "negative price must be less than positive");
    }

    void testSmallestRepresentableIncrement()
    {
        constexpr Price price { 6'500'000'000'000 };
        constexpr Price tick { 1 };
        constexpr Price result = price + tick;

        Assert(result.raw() == price.raw() + 1,"price must preserve smallest representable increment");
    }

    void testNegativeRawValue()
    {
        constexpr Price price { -123'456'789 };

        Assert(price.raw() == -123'456'789, "negative raw value must be preserved");
        Assert(!price.isZero(), "negative price must not be zero");
        Assert(!price.isPositive(), "negative price must not be positive");
    }

    void testLargeRawValue()
    {
        constexpr Price price { std::numeric_limits<int64_t>::max() };

        Assert(price.raw() == std::numeric_limits<int64_t>::max(),"maximum int64 raw value must be preserved");
        Assert(price.isPositive(), "maximum int64 price must be positive");
    }

    void testMinimumRawValue()
    {
        constexpr Price price { std::numeric_limits<int64_t>::min() };

        Assert(price.raw() == std::numeric_limits<int64_t>::min(),"minimum int64 raw value must be preserved");
        Assert(!price.isZero(), "minimum int64 price must not be zero");
        Assert(!price.isPositive(), "minimum int64 price must not be positive");
    }

    void testFromIntegerZero()
    {
        constexpr Price price = Price::fromInteger(0);

        Assert(price.isZero(), "fromInteger zero must produce zero price");
        Assert(price.raw() == 0, "fromInteger zero raw value must be zero");
    }

    void testFromIntegerPositive()
    {
        constexpr auto price = Price::fromInteger(65'000);
        Assert(price == Price { 6'500'000'000'000 },"fromInteger must create correctly scaled price");
    }

    void testFromIntegerNegative()
    {
        constexpr auto price = Price::fromInteger(-10);
        Assert(price == Price { -1'000'000'000 },"fromInteger must correctly scale negative value");
    }

    void testArithmeticPreservesScale()
    {
        constexpr Price price = Price::fromInteger(100);
        constexpr Price increment { 50'000'000 };
        constexpr Price result = price + increment;

        Assert(result == Price { 10'050'000'000 },"arithmetic must operate on fixed-point raw representation");
    }

        void testMinimumPriceIncrement()
    {
        constexpr Price price { Price::Scale };
        constexpr Price tick { 1 };
        constexpr Price nextPrice = price + tick;
        
        Assert(nextPrice.raw() - price.raw() == 1,"minimum price increment must be exactly one raw unit");
    }

    void testRepeatedTickUpdates()
    {
        constexpr Price initialPrice { Price::Scale };
        constexpr Price tick { 1 };
        constexpr uint32_t UpdatesCount { 1'000 };

        Price price = initialPrice;

        for (uint32_t index { 0 }; index < UpdatesCount; ++index)
            price += tick;

        Assert(price.raw() == initialPrice.raw() + UpdatesCount * tick.raw(),
            "repeated tick updates must preserve exact price value");
    }

    void testPriceRoundTrip()
    {
        constexpr Price::Value rawValue { 6'500'000'123'456 };
        constexpr Price original { rawValue };
        constexpr Price copy { original.raw() };

        Assert(copy == original, "price reconstructed from raw value must be identical");
        Assert(copy.raw() == rawValue, "round trip must preserve raw value exactly");
    }

    void testLargeCryptoPrice()
    {
        constexpr Price price { 123'456'789'123'456'789 };

        Assert(price.isPositive(), "large crypto price must be positive");
        Assert(price.raw() == 123'456'789'123'456'789,"large crypto price must preserve exact raw value");
    }

    void testVerySmallPrice()
    {
        constexpr Price minimumPrice { 1 };

        Assert(minimumPrice.isPositive(), "minimum non-zero price must be positive");
        Assert(!minimumPrice.isZero(), "minimum non-zero price must not be zero");
        Assert(minimumPrice.raw() == 1, "minimum price must preserve raw value");
    }

    void testBidAskSpread()
    {
        constexpr Price bid { 6'500'000'000'000 };
        constexpr Price ask { 6'500'000'000'100 };
        constexpr Price spread = ask - bid;

        Assert(spread.isPositive(), "normal bid ask spread must be positive");
        Assert(spread.raw() == 100, "bid ask spread must have correct value");
    }

    void testMidPrice()
    {
        constexpr Price bid { 6'500'000'000'000 };
        constexpr Price ask { 6'500'000'000'200 };
        constexpr Price::Value midRaw = (bid.raw() + ask.raw()) / 2;
        constexpr Price midPrice { midRaw };

        Assert(midPrice > bid, "mid price must be greater than bid");
        Assert(midPrice < ask, "mid price must be less than ask");
        Assert(midPrice.raw() == 6'500'000'000'100,"mid price must have correct value");
    }

    void testMidPriceRounding()
    {
        constexpr Price bid { 6'500'000'000'000 };
        constexpr Price ask { 6'500'000'000'001 };
        constexpr Price::Value midRaw = (bid.raw() + ask.raw()) / 2;
        constexpr Price midPrice { midRaw };

        Assert(midPrice.raw() == bid.raw(),"mid price with odd spread must truncate toward zero");
        Assert(midPrice >= bid && midPrice <= ask,"rounded mid price must remain inside bid ask range");
    }

    void testPriceOrderingForOrderBook()
    {
        constexpr Price lowerPrice { 6'499'999'999'999 };
        constexpr Price middlePrice { 6'500'000'000'000 };
        constexpr Price higherPrice { 6'500'000'000'001 };

        Assert(lowerPrice < middlePrice, "lower price must compare less than middle price");
        Assert(middlePrice < higherPrice, "middle price must compare less than higher price");
        Assert(higherPrice > middlePrice, "higher price must compare greater than middle price");
        Assert(middlePrice > lowerPrice, "middle price must compare greater than lower price");
        Assert(lowerPrice <= middlePrice, "lower price must be less than or equal to middle price");
        Assert(middlePrice >= lowerPrice, "middle price must be greater than or equal to lower price");
        Assert(lowerPrice != middlePrice,"different price levels must not compare equal");
        Assert(middlePrice == Price { 6'500'000'000'000 },"equal price levels must compare equal");
    }

    void testNegativePriceDifference()
    {
        constexpr Price higherPrice { 6'500'000'000'000 }, lowerPrice { 6'499'999'999'900 };
        constexpr Price difference = lowerPrice - higherPrice;

        Assert(!difference.isPositive(),"negative price difference must not be positive");
        //Assert(difference.isZero(),"negative price difference must not be zero");
        Assert(difference.raw() == -100,"negative price difference must preserve its sign and value");
    }
}

void price_test()
{
    testDefaultConstruction();
    testRawConstruction();
    testScale();

    testFromInteger();
    testFromIntegerZero();
    testFromIntegerPositive();
    testFromIntegerNegative();

    testZeroPrice();
    testPositivePrice();
    testNegativePrice();
    testNegativeRawValue();

    testAddition();
    testAdditionWithZero();
    testSubtraction();
    testSubtractionWithZero();
    testMultiplication();
    testMultiplicationByZero();
    testMultiplicationByNegativeValue();
    testAdditionAssignment();
    testAdditionAssignmentWithZero();
    testSubtractionAssignment();
    testSubtractionAssignmentWithZero();

    testEquality();
    testLessComparison();
    testLessOrEqualComparison();
    testGreaterComparison();
    testGreaterOrEqualComparison();
    testOrdering();

    testSmallestRepresentableIncrement();
    testArithmeticPreservesScale();

    testLargeRawValue();
    testMinimumRawValue();

    testMinimumPriceIncrement();
    testRepeatedTickUpdates();
    testPriceRoundTrip();
    testLargeCryptoPrice();
    testVerySmallPrice();
    testBidAskSpread();
    testMidPrice();
    testMidPriceRounding();
    testPriceOrderingForOrderBook();
    testNegativePriceDifference();

    std::cout << "All Price tests: OK\n";
}