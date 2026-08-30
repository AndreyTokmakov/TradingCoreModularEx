/**============================================================================
Name        : execution_work_item.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Work items processed by the execution worker.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_WORK_ITEM_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_WORK_ITEM_HPP

#include "execution_report.hpp"
#include "order.hpp"

#include <variant>

namespace trading::execution
{
    using ExecutionWorkItem = std::variant<OrderRequest, ExecutionReport>;
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_WORK_ITEM_HPP