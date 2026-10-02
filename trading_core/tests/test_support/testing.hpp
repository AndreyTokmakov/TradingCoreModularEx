/**============================================================================
Name        : testing.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : testing.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TESTING_HPP
#define FINANCETECHNOLOGYPROJECTS_TESTING_HPP

#include <source_location>
#include <iostream>
#include <string_view>

namespace testing
{
    enum class Action
    {
        Continue,
        Terminate
    };

    constexpr void performAction(const Action action)
    {
        if (Action::Terminate == action)
        {
            std::terminate();
        }
    }

    template<typename Ty>
    concept Comparable = std::equality_comparable<Ty>;

    template<typename Ty>
    concept Streamable = requires(std::ostream& stream, const Ty& value)
    {
        stream << value;
    };

    template<typename Ty>
    concept Ordered = std::totally_ordered<Ty>;

    template<typename Ty>
    concept FloatingPoint = std::floating_point<Ty>;

    template<typename Ty>
    concept HasEmpty = requires(const Ty& value)
    {
        { value.empty() } -> std::convertible_to<bool>;
    };

    template<typename Ty>
    concept HasSized = requires(const Ty& value)
    {
        { value.size() } -> std::convertible_to<std::size_t>;
    };

    template<typename Ty>
    concept ExpectedType = requires
    {
        typename Ty::value_type;
        typename Ty::error_type;
    };

    inline void printLocation(const std::source_location& location)
    {
        std::println(std::cerr, "\tFile: {}\n\tFunction: {}\n\tLine: {}",
                     location.file_name(), location.function_name(), location.line());
    }

    inline void reportFailure(const std::string_view message,
                              const Action action,
                              const std::source_location location)
    {
        std::println(std::cerr, "{}", message);
        printLocation(location);
        performAction(action);
    }

    template<typename Expected, typename Actual>
    void reportComparisonFailure(const std::string_view message,
                                 const Expected& expected,
                                 const Actual& actual,
                                 const Action action,
                                 const std::source_location location)
    {
        if constexpr (Streamable<Expected> && Streamable<Actual>) {
            std::println(std::cerr, "{}\n\tExpected: {}\n\tActual:   {}", message, expected, actual);
        } else {
            std::println(std::cerr, "{}", message);
        }

        printLocation(location);
        performAction(action);
    }

    template<typename Expected, typename Actual>
        requires std::equality_comparable_with<Expected, Actual>
    void AssertEqual(const Expected& expected,
                     const Actual& actual,
                     const std::string_view message,
                     const Action action = Action::Terminate,
                     const std::source_location location = std::source_location::current())
    {
        if (expected != actual)
        {
            reportComparisonFailure(message, expected, actual, action, location);
        }
    }

    template<typename Expected, typename Actual>
        requires std::equality_comparable_with<Expected, Actual>
    void AssertEqual(const Expected& expected,
                     const Actual& actual,
                     const Action action = Action::Terminate,
                     const std::source_location location = std::source_location::current())
    {
        AssertEqual(expected, actual, "Assertion failed:", action, location);
    }

    template<typename Expected, typename Actual>
        requires std::equality_comparable_with<Expected, Actual>
    void AssertNotEqual(const Expected& expected,
                       const Actual& actual,
                       const std::string_view message,
                       const Action action = Action::Terminate,
                       const std::source_location location = std::source_location::current())
    {
        if (expected == actual)
        {
            reportComparisonFailure(message, expected, actual, action, location);
        }
    }

    template<typename Expected, typename Actual>
        requires std::equality_comparable_with<Expected, Actual>
    void AssertNotEqual(const Expected& expected,
                       const Actual& actual,
                       const Action action = Action::Terminate,
                       const std::source_location location = std::source_location::current())
    {
        AssertNotEqual(expected, actual, "Assertion failed:", action, location);
    }

    template<typename Ty>
    void AssertNotNull(const Ty* actual,
                       const std::string_view message,
                       const Action action = Action::Terminate,
                       const std::source_location location = std::source_location::current())
    {
        if (nullptr == actual)
        {
            reportFailure(message, action, location);
        }
    }

    template<typename Ty>
    void AssertNotNull(const Ty* actual,
                       const Action action = Action::Terminate,
                       const std::source_location location = std::source_location::current())
    {
        AssertNotNull(actual, "Assertion failed (Actual value is null)", action, location);
    }

    template<typename Ty>
    void AssertNull(const Ty* actual,
                    const std::string_view message,
                    const Action action = Action::Terminate,
                    const std::source_location location = std::source_location::current())
    {
        if (nullptr != actual)
        {
            reportFailure(message, action, location);
        }
    }

    template<typename Ty>
    void AssertNull(const Ty* actual,
                    const Action action = Action::Terminate,
                    const std::source_location location = std::source_location::current())
    {
        AssertNull(actual, "Assertion failed (Actual value is not null)", action, location);
    }

    inline void AssertTrue(const bool condition,
                           const std::string_view message,
                           const Action action = Action::Terminate,
                           const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            reportFailure(message, action, location);
        }
    }

    inline void AssertTrue(const bool condition,
                           const Action action = Action::Terminate,
                           const std::source_location location = std::source_location::current())
    {
        AssertTrue(condition, "Condition is False (True expected)", action, location);
    }

    inline void Assert(const bool condition,
                       const std::string_view message,
                       const std::source_location& location = std::source_location::current())
    {
        AssertTrue(condition, message, Action::Terminate, location);
    }

    inline void AssertFalse(const bool condition,
                            const std::string_view message,
                            const Action action = Action::Terminate,
                            const std::source_location location = std::source_location::current())
    {
        if (condition)
        {
            reportFailure(message, action, location);
        }
    }

    inline void AssertFalse(const bool condition,
                            const Action action = Action::Terminate,
                            const std::source_location location = std::source_location::current())
    {
        AssertFalse(condition, "Condition is True (False expected)", action, location);
    }

    template<typename Actual, typename Expected>
        requires std::totally_ordered_with<Actual, Expected>
    void AssertGreater(const Actual& actual,
                       const Expected& expected,
                       const std::string_view message,
                       const Action action = Action::Terminate,
                       const std::source_location location = std::source_location::current())
    {
        if (actual <= expected)
        {
            reportComparisonFailure(message, expected, actual, action, location);
        }
    }

    template<typename Actual, typename Expected>
        requires std::totally_ordered_with<Actual, Expected>
    void AssertGreater(const Actual& actual,
                       const Expected& expected,
                       const Action action = Action::Terminate,
                       const std::source_location location = std::source_location::current())
    {
        AssertGreater(actual, expected, "Assertion failed:", action, location);
    }

    template<typename Actual, typename Expected>
        requires std::totally_ordered_with<Actual, Expected>
    void AssertGreaterEqual(const Actual& actual,
                            const Expected& expected,
                            const std::string_view message,
                            const Action action = Action::Terminate,
                            const std::source_location location = std::source_location::current())
    {
        if (actual < expected)
        {
            reportComparisonFailure(message, expected, actual, action, location);
        }
    }

    template<typename Actual, typename Expected>
        requires std::totally_ordered_with<Actual, Expected>
    void AssertGreaterEqual(const Actual& actual,
                            const Expected& expected,
                            const Action action = Action::Terminate,
                            const std::source_location location = std::source_location::current())
    {
        AssertGreaterEqual(actual, expected, "Assertion failed:", action, location);
    }

    template<typename Actual, typename Expected>
        requires std::totally_ordered_with<Actual, Expected>
    void AssertLess(const Actual& actual,
                    const Expected& expected,
                    const std::string_view message,
                    const Action action = Action::Terminate,
                    const std::source_location location = std::source_location::current())
    {
        if (actual >= expected)
        {
            reportComparisonFailure(message, expected, actual, action, location);
        }
    }

    template<typename Actual, typename Expected>
        requires std::totally_ordered_with<Actual, Expected>
    void AssertLess(const Actual& actual,
                    const Expected& expected,
                    const Action action = Action::Terminate,
                    const std::source_location location = std::source_location::current())
    {
        AssertLess(actual, expected, "Assertion failed:", action, location);
    }

    template<typename Actual, typename Expected>
        requires std::totally_ordered_with<Actual, Expected>
    void AssertLessEqual(const Actual& actual,
                         const Expected& expected,
                         const std::string_view message,
                         const Action action = Action::Terminate,
                         const std::source_location location = std::source_location::current())
    {
        if (actual > expected)
        {
            reportComparisonFailure(message, expected, actual, action, location);
        }
    }

    template<typename Actual, typename Expected>
        requires std::totally_ordered_with<Actual, Expected>
    void AssertLessEqual(const Actual& actual,
                         const Expected& expected,
                         const Action action = Action::Terminate,
                         const std::source_location location = std::source_location::current())
    {
        AssertLessEqual(actual, expected, "Assertion failed:", action, location);
    }

    template<FloatingPoint Ty>
    void AssertNear(const Ty& expected,
                    const Ty& actual,
                    const Ty& tolerance,
                    const std::string_view message,
                    const Action action = Action::Terminate,
                    const std::source_location location = std::source_location::current())
    {
        if (std::abs(expected - actual) > tolerance)
        {
            reportComparisonFailure(message, expected, actual, action, location);
        }
    }

    template<FloatingPoint Ty>
    void AssertNear(const Ty& expected,
                    const Ty& actual,
                    const Ty& tolerance,
                    const Action action = Action::Terminate,
                    const std::source_location location = std::source_location::current())
    {
        AssertNear(expected, actual, tolerance, "Assertion failed:", action, location);
    }

    template<ExpectedType Ty>
    void AssertExpected(const Ty& actual,
                        const std::string_view message,
                        const Action action = Action::Terminate,
                        const std::source_location location = std::source_location::current())
    {
        if (!actual.has_value())
        {
            reportFailure(message, action, location);
        }
    }

    template<ExpectedType Ty>
    void AssertExpected(const Ty& actual,
                        const Action action = Action::Terminate,
                        const std::source_location location = std::source_location::current())
    {
        AssertExpected(actual, "Assertion failed (Expected value, but got error)", action, location);
    }

    template<ExpectedType Ty>
        requires Comparable<typename Ty::value_type>
    void AssertExpected(const Ty& actual,
                        const typename Ty::value_type& expected,
                        const std::string_view message,
                        const Action action = Action::Terminate,
                        const std::source_location location = std::source_location::current())
    {
        if (!actual.has_value())
        {
            reportFailure(message, action, location);
            return;
        }

        AssertEqual(expected, actual.value(), message, action, location);
    }

    template<ExpectedType Ty>
        requires Comparable<typename Ty::value_type>
    void AssertExpected(const Ty& actual,
                        const typename Ty::value_type& expected,
                        const Action action = Action::Terminate,
                        const std::source_location location = std::source_location::current())
    {
        AssertExpected(actual, expected, "Assertion failed:", action, location);
    }

    template<ExpectedType Ty>
        requires Comparable<typename Ty::error_type>
    void AssertExpectedError(const Ty& actual,
                             const typename Ty::error_type& expected,
                             const std::string_view message,
                             const Action action = Action::Terminate,
                             const std::source_location location = std::source_location::current())
    {
        if (actual.has_value())
        {
            reportFailure(message, action, location);
            return;
        }

        AssertEqual(expected, actual.error(), message, action, location);
    }

    template<ExpectedType Ty>
        requires Comparable<typename Ty::error_type>
    void AssertExpectedError(const Ty& actual,
                             const typename Ty::error_type& expected,
                             const Action action = Action::Terminate,
                             const std::source_location location = std::source_location::current())
    {
        AssertExpectedError(actual, expected, "Assertion failed:", action, location);
    }

    template<HasEmpty Ty>
    void AssertEmpty(const Ty& actual,
                     const std::string_view message,
                     const Action action = Action::Terminate,
                     const std::source_location location = std::source_location::current())
    {
        if (!actual.empty())
        {
            if constexpr (HasSized<Ty>)
            {
                std::println(std::cerr, "{}. Actual size = {}", message, actual.size());
            }
            else
            {
                std::println(std::cerr, "{}", message);
            }
        }
        printLocation(location);
        performAction(action);
    }

    template<HasEmpty Ty>
    void AssertEmpty(const Ty& actual,
                     const Action action = Action::Terminate,
                     const std::source_location location = std::source_location::current())
    {
        AssertEmpty(actual, "Assertion failed (Collection is not empty)", action, location);
    }

    template<HasEmpty Ty>
    void AssertNotEmpty(const Ty& actual,
                        const std::string_view message,
                        const Action action = Action::Terminate,
                        const std::source_location location = std::source_location::current())
    {
        if (actual.empty())
        {
            reportFailure(message, action, location);
        }
    }

    template<HasEmpty Ty>
    void AssertNotEmpty(const Ty& actual,
                        const Action action = Action::Terminate,
                        const std::source_location location = std::source_location::current())
    {
        AssertNotEmpty(actual, "Assertion failed (Collection is empty)", action, location);
    }

    template<HasSized Ty>
    void AssertSize(const Ty& actual,
                    const std::size_t expected,
                    const std::string_view message,
                    const Action action = Action::Terminate,
                    const std::source_location location = std::source_location::current())
    {
        if (actual.size() != expected)
        {
            reportComparisonFailure(message, expected, actual.size(), action, location);
        }
    }

    template<HasSized Ty>
    void AssertSize(const Ty& actual,
                    const std::size_t expected,
                    const Action action = Action::Terminate,
                    const std::source_location location = std::source_location::current())
    {
        AssertSize(expected, actual, "Assertion failed:", action, location);
    }
}

#endif //FINANCETECHNOLOGYPROJECTS_TESTING_HPP