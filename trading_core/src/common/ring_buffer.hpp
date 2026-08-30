/**============================================================================
Name        : condition_variable_queue.hpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Mutex and condition-variable based Queue implementation.
============================================================================**/


#ifndef FINANCETECHNOLOGYPROJECTS_RING_BUFFER_HPP
#define FINANCETECHNOLOGYPROJECTS_RING_BUFFER_HPP

#include <atomic>
#include <vector>

namespace trading::concurrency
{
#ifdef __cpp_lib_hardware_interference_size
    inline constexpr std::size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
    inline constexpr std::size_t kCacheLineSize = 64;
#endif

    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & (value - 1)));
    }

    template<typename Ty, uint32_t Capacity>
    class RingBuffer
    {
        using size_type  = int64_t;
        using value_type = Ty;
        using collection_type = std::vector<value_type>;

        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");
        static_assert(is_pow_of_2(Capacity), "ERROR: Capacity must be a power of 2");
        static_assert(Capacity >= 2, "Buffer size must be at least 2");

    public:
        RingBuffer() = default;

        RingBuffer(const RingBuffer&) = delete;
        RingBuffer& operator=(const RingBuffer&) = delete;

        template <typename... Args>
        bool emplace(Args&&... args)
        {
            const size_type seq = writeCursor.load(std::memory_order_relaxed);
            /** Full when write is exactly N ahead of read **/
            if (seq - cachedReadCursor >= static_cast<size_type>(Capacity))
            {
                cachedReadCursor = readCursor.load(std::memory_order_acquire);
                if (seq - cachedReadCursor >= static_cast<size_type>(Capacity)) {
                    return false; // full
                }
            }

            // Construct the element directly in the slot
            const std::size_t idx = seq & kMask;
            buffer[idx].~value_type();
            new (&buffer[idx]) value_type(std::forward<Args>(args)...);

            // Publish: make the write visible to the consumer
            writeCursor.store(seq + 1, std::memory_order_release);
            return true;
        }

        bool push(const value_type& item)
        {
            const size_type seq = writeCursor.load(std::memory_order_relaxed);
            /** Full when write is exactly N ahead of read **/
            if (seq - cachedReadCursor >= static_cast<size_type>(Capacity))
            {
                cachedReadCursor = readCursor.load(std::memory_order_acquire);
                if (seq - cachedReadCursor >= static_cast<size_type>(Capacity)) {
                    return false; // full
                }
            }

            // Construct the element directly in the slot
            const std::size_t idx = seq & kMask;
            buffer[idx] = item;

            // Publish: make the write visible to the consumer
            writeCursor.store(seq + 1, std::memory_order_release);
            return true;
        }

        [[nodiscard]]
        bool pop(value_type& item)
        {
            const size_type seq = readCursor.load(std::memory_order_relaxed);

            // Empty when read has caught up to write
            if (seq >= cachedWriteCursor)
            {
                // Refresh our cached copy of the write cursor
                cachedWriteCursor =writeCursor.load(std::memory_order_acquire);
                if (seq >= cachedWriteCursor) {
                    return false;
                }
            }

            // Read from the slot
            const std::size_t idx = seq & kMask;
            item = std::move(buffer[idx]);

            // Advance read cursor
            readCursor.store(seq + 1, std::memory_order_release);
            return true;
        }

    private:
        static constexpr std::size_t kMask { Capacity - 1 };

        alignas(kCacheLineSize) std::array<value_type, Capacity> buffer{};

        // Producer's cache line
        alignas(kCacheLineSize) std::atomic<size_type> writeCursor { 0 };
        size_type cachedReadCursor { 0 };  // producer's local cache

        // Consumer's cache line
        alignas(kCacheLineSize) std::atomic<size_type> readCursor { 0 };
        size_type cachedWriteCursor { 0 }; // consumer's local cache
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RING_BUFFER_HPP
