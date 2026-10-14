/**============================================================================
Name        : common.hpp
Created on  : 09.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : common.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_COMMON_HPP
#define TRADINGCOREMODULAREX_COMMON_HPP

#include "common/condition_variable_queue.hpp"
#include "common/worker.hpp"

namespace trading::common
{
    template <typename Ty>
    using Queue = concurrency::Queue<Ty>;

    template <typename Ty>
    using CVQueue = concurrency::ConditionVariableQueue<Ty>;
}

#endif //TRADINGCOREMODULAREX_COMMON_HPP
