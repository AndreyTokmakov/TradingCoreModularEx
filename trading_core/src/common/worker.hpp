/**============================================================================
Name        : worker.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Queue abstraction used for inter-thread communication.
============================================================================**/

#ifndef TRADINGCOREBASE_WORKER_HPP
#define TRADINGCOREBASE_WORKER_HPP

#include <atomic>
#include <thread>

namespace trading::common
{
    template<typename Derived>
    struct Worker
    {
        Worker() = default;

        ~Worker() {
            stop();
        }

        Worker(const Worker&) = delete;
        Worker& operator=(const Worker&) = delete;

        Worker(Worker&&) = delete;
        Worker& operator=(Worker&&) = delete;

        void start()
        {
            if (running.load(std::memory_order_relaxed))
                return;
            running.store(true, std::memory_order_relaxed);
            thread = std::jthread { &Worker::execute, this };
        }

        void stop() noexcept
        {
            if (!running.load(std::memory_order_relaxed))
                return;
            if (thread.joinable())
                thread.join();
            running.store(false, std::memory_order_relaxed);
        }

    private:

        void execute() {
            return static_cast<Derived*>(this)->run();
        }

        std::jthread thread;
        std::atomic_bool running { false };
    };
}

#endif //TRADINGCOREBASE_WORKER_HPP

