/**============================================================================
Name        : queue.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Queue abstraction used for inter-thread communication.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_QUEUE_HPP
#define FINANCETECHNOLOGYPROJECTS_QUEUE_HPP

namespace trading::concurrency
{
    template<typename T>
    class Queue
    {
    public:
        virtual ~Queue() = default;

        virtual void push(T value) = 0;

        [[nodiscard]]
        virtual bool waitPop(T& value) = 0;

        virtual void close() noexcept = 0;

        [[nodiscard]]
        virtual bool isClosed() const noexcept = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_QUEUE_HPP