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
        if ( Action::Terminate == action)
        {
            std::terminate();
        }
    }

    constexpr void printLocation(const std::source_location& location)
    {
        std::println(std::cerr, "\tFile: {}\n\tFunction: {}\n\tLine: {}",
            location.file_name(), location.function_name(), location.line());
    }

    constexpr void Assert(const bool condition,
                          const std::string_view message,
                          const std::source_location& location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << message << '\n';
            printLocation(location);
            std::terminate();
        }
    }

    // TODO: Add concepts to 'Ty'
    template<typename Ty>
    constexpr void AssertEqual(const Ty& expected,
                               const Ty& actual,
                               const std::string_view message,
                               const Action action = Action::Terminate,
                               const std::source_location& location = std::source_location::current())
    {
        if (expected != actual)
        {
            std::println(std::cerr, "{}: {} != {}", message, expected, actual);
            printLocation(location);
            performAction(action);
        }
    }

    // TODO: Add concepts to 'Ty'
    template<typename Ty>
    constexpr void AssertEqual(const Ty& expected,
                               const Ty& actual,
                               const Action action = Action::Terminate,
                               const std::source_location& location = std::source_location::current())
    {
        AssertEqual(expected, actual, "Assertion failed:", action, location);
    }

    // TODO: Add concepts to 'Ty'
    template<typename Ty>
    constexpr void AssertNotNull(const Ty* actual,
                                 const std::string_view message,
                                 const Action action = Action::Terminate,
                                 const std::source_location& location = std::source_location::current())
    {
        if (nullptr == actual)
        {
            std::println(std::cerr, "{}", message);
            printLocation(location);
            performAction(action);
        }
    }

    // TODO: Add concepts to 'Ty'
    template<typename Ty>
    constexpr void AssertNotNull(const Ty* actual,
                                 const Action action = Action::Terminate,
                                 const std::source_location& location = std::source_location::current())
    {
        AssertNotNull(actual, "Assertion failed (Actual value is null)", action, location);
    }

    // TODO: Add concepts to 'Ty'
    template<typename Ty>
    constexpr void AssertIsNull(const Ty* actual,
                                const std::string_view message,
                                const Action action = Action::Terminate,
                                const std::source_location& location = std::source_location::current())
    {
        if (nullptr != actual)
        {
            std::println(std::cerr, "{}",message);
            printLocation(location);
            performAction(action);
        }
    }

    // TODO: Add concepts to 'Ty'
    template<typename Ty>
    constexpr void AssertIsNull(const Ty* actual,
                                const Action action = Action::Terminate,
                                const std::source_location& location = std::source_location::current())
    {
        AssertIsNull(actual, "Assertion failed (Actual value is not null)", action, location);
    }

    constexpr void AssertTrue(const bool condition,
                              const std::string_view message,
                              const Action action = Action::Terminate,
                              const std::source_location& location = std::source_location::current())
    {
        if (!condition)
        {
            std::println(std::cerr, "{}", message);
            printLocation(location);
            performAction(action);
        }
    }

    constexpr void AssertTrue(const bool condition,
                              const Action action = Action::Terminate,
                              const std::source_location& location = std::source_location::current())
    {
        AssertTrue(condition, "Condition is False (True expected):", action, location);
    }

    constexpr void AssertFalse(const bool condition,
                               const std::string_view message,
                               const Action action = Action::Terminate,
                               const std::source_location& location = std::source_location::current())
    {
        if (condition)
        {
            std::println(std::cerr, "{}", message);
            printLocation(location);
            performAction(action);
        }
    }

    constexpr void AssertFalse(const bool condition,
                               const Action action = Action::Terminate,
                               const std::source_location& location = std::source_location::current())
    {
        AssertFalse(condition, "Condition is True (False expected):", action, location);
    }
}

#endif //FINANCETECHNOLOGYPROJECTS_TESTING_HPP