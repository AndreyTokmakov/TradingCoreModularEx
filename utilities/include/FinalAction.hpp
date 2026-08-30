/**============================================================================
Name        : FinalAction.hpp
Created on  : 19.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : FinalAction.hpp
============================================================================**/

#ifndef CPPPROJECTS_FINALACTION_HPP
#define CPPPROJECTS_FINALACTION_HPP

#include <concepts>
#include <utility>
#include <cstdint>
#include <exception>

namespace utilities::final_action
{
    template<typename Fn>
    struct [[nodiscard]] ScopeExit
    {
        template<typename Func>
        requires std::constructible_from<Fn, Func&&> &&
                 std::is_nothrow_constructible_v<Fn, Func&&>
                 // && std::is_nothrow_invocable_v<Fn>
        constexpr explicit ScopeExit(Func&& func) noexcept(std::is_nothrow_constructible_v<Fn, Func&&>)
            : exitFunction ( std::forward<Func>(func) ) {
        }

        ScopeExit(const ScopeExit&) = delete;
        ScopeExit(ScopeExit&&) noexcept = delete;

        ScopeExit& operator=(const ScopeExit&) = delete;
        ScopeExit& operator=(ScopeExit&&) noexcept = delete;

        constexpr ~ScopeExit() noexcept
        {
            if (callOnExit) {
                exitFunction();
            }
        }

        constexpr void release() noexcept {
            callOnExit = false;
        }

    private:

        bool callOnExit { true };

        [[no_unique_address]]
        Fn exitFunction;
    };

    template<typename Func>
    ScopeExit(Func) -> ScopeExit<std::decay_t<Func>>;

    template<class Func>
    [[nodiscard]]
    ScopeExit<Func> scopeExit(Func&& callable) noexcept(std::is_nothrow_constructible_v<std::decay_t<Func>, Func&&>)
    {
        return ScopeExit<std::decay_t<Func>>(std::forward<Func>(callable));
    }
}


namespace final_action
{
    template<typename Fn>
    struct [[nodiscard]] ScopeFail
    {
        template<typename Func>
        requires std::constructible_from<Fn, Func&&> &&
                 std::is_nothrow_constructible_v<Fn, Func&&>
        explicit ScopeFail(Func&& func) noexcept(std::is_nothrow_constructible_v<Fn, Func&&>)
            : exceptionCount(std::uncaught_exceptions()), exitFunction(std::forward<Func>(func))
        {}

        ScopeFail(const ScopeFail&) = delete;
        ScopeFail(ScopeFail&&) noexcept = delete;

        ScopeFail& operator=(const ScopeFail&) = delete;
        ScopeFail& operator=(ScopeFail&&) noexcept = delete;

        ~ScopeFail() noexcept
        {
            if (callOnFail && std::uncaught_exceptions() > exceptionCount) {
                exitFunction();
            }
        }

        void release() noexcept{
            callOnFail = false;
        }

        private:

            bool callOnFail { true };
            uint8_t exceptionCount { 0 };

            [[no_unique_address]]
            Fn exitFunction;
    };

    template<class F>
    ScopeFail(F) -> ScopeFail<std::decay_t<F>>;

    template<class Func>
    [[nodiscard]]
    ScopeFail<Func> scopeFail(Func&& fn) noexcept(std::is_nothrow_constructible_v<std::decay_t<Func>, Func&&>)
    {
        return ScopeFail<std::decay_t<Func>>(std::forward<Func>(fn));
    }
}

namespace final_action
{
    template<typename RollbackFn, typename CommitFn>
    struct [[nodiscard]] TransactionalScope
    {
        template<typename RollBack, typename Commit>
        requires std::constructible_from<RollbackFn, RollBack&&> &&
                 std::constructible_from<CommitFn, Commit&&>
                 // &&  std::is_nothrow_invocable_v<RollbackFn> && std::is_nothrow_invocable_v<CommitFn>
        TransactionalScope(RollBack&& rollback, Commit&& commit)
            noexcept(std::is_nothrow_constructible_v<RollbackFn, RollBack&&> &&
                     std::is_nothrow_constructible_v<CommitFn, Commit&&>)
            : exceptionCount(std::uncaught_exceptions())
            , rollbackFn(std::forward<RollBack>(rollback))
            , commitFn(std::forward<Commit>(commit))
        {}

        TransactionalScope(const TransactionalScope&) = delete;
        TransactionalScope& operator=(const TransactionalScope&) = delete;

        TransactionalScope(TransactionalScope&&) = delete;
        TransactionalScope& operator=(TransactionalScope&&) = delete;

        ~TransactionalScope() noexcept
        {
            if (!active)
                return;
            if (std::uncaught_exceptions() > exceptionCount)
                rollbackFn();
            else
                commitFn();
        }

        void commit() noexcept
        {
            active = false;
            commitFn();
        }

        void rollback() noexcept
        {
            active = false;
            rollbackFn();
        }

    private:

        bool active { true };
        uint8_t exceptionCount { 0 };

        [[no_unique_address]]
        RollbackFn rollbackFn;

        [[no_unique_address]]
        CommitFn   commitFn;
    };

    template<typename RollbackFn, typename CommitFn>
    TransactionalScope(RollbackFn, CommitFn)-> TransactionalScope<std::decay_t<RollbackFn>, std::decay_t<CommitFn>>;

    template<class RollbackFn, class CommitFn>
    [[nodiscard]]
    auto transactionalScope(RollbackFn&& rollback, CommitFn&& commit)
    {
        return TransactionalScope<std::decay_t<RollbackFn>, std::decay_t<CommitFn>>(
            std::forward<RollbackFn>(rollback), std::forward<CommitFn>(commit)
        );
    }
}

#endif //CPPPROJECTS_FINALACTION_HPP