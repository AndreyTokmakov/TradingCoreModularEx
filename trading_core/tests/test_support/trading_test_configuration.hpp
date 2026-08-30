/**============================================================================
Name        : trading_test_configuration.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trading_test_configuration.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TRADING_TEST_CONFIGURATION_HPP
#define FINANCETECHNOLOGYPROJECTS_TRADING_TEST_CONFIGURATION_HPP

#include "quantity.hpp"
#include "types.hpp"

namespace trading::testing
{
    struct TradingTestConfiguration
    {
        InstrumentId instrument { 1 };
        Quantity orderQuantity { 1 };
        int64_t thresholdNumerator { 7 };
        int64_t thresholdDenominator { 10 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TRADING_TEST_CONFIGURATION_HPP
