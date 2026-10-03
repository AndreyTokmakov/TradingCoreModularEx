/**============================================================================
Name        : sequence_range.hpp
Created on  : 28.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : sequence_range.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_SEQUENCE_RANGE_HPP
#define TRADINGCOREMODULAREX_SEQUENCE_RANGE_HPP

#include "core/types.hpp"

namespace trading::market_data
{
    struct SequenceRange
    {
        SequenceNumber first { 0 };
        SequenceNumber last { 0 };
    };
};

#endif //TRADINGCOREMODULAREX_SEQUENCE_RANGE_HPP
