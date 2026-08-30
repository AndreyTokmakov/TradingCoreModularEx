/**============================================================================
Name        : condition_variable_queue.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Mutex and condition-variable based Queue implementation.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_CONDITION_VARIABLE_QUEUE_HPP
#define FINANCETECHNOLOGYPROJECTS_CONDITION_VARIABLE_QUEUE_HPP

#include "queue.hpp"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

namespace trading::concurrency
{
    template<typename T>
    class ConditionVariableQueue final : public Queue<T>
    {
    public:
        void push(T value) override
        {
            {
                std::lock_guard lock { mutex };
                if (closed)
                    return;
                values.push_back(std::move(value));
            }
            condition.notify_one();
        }

        [[nodiscard]]
        bool waitPop(T& value) override
        {
            std::unique_lock lock { mutex };
            condition.wait(lock, [this] {
                return closed || !values.empty();
            });

            if (values.empty())
                return false;

            value = std::move(values.front());
            values.pop_front();

            return true;
        }

        void close() noexcept override
        {
            {
                std::lock_guard lock { mutex };
                closed = true;
            }
            condition.notify_all();
        }

        [[nodiscard]]
        bool isClosed() const noexcept override
        {
            std::lock_guard lock { mutex };
            return closed;
        }

    private:
        mutable std::mutex mutex;
        std::condition_variable condition;
        std::deque<T> values;
        bool closed { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_CONDITION_VARIABLE_QUEUE_HPP