//
// One-shot bind (deferred call) implementation.
// Using perfect forwarding and move semantics it is
// possible to defer a function call even until the 
// original objects' lifetimes have ended.
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// Licnese: Please see LICENSE.md
//

#ifndef DEFER_HPP
#define DEFER_HPP

#include <tuple>
#include <type_traits>

#include "eval.hpp"
#include "type_support/function_traits.hpp"

namespace fnk
{
    //
    // Non-Tuple Versions
    //
    template <class F, typename ... Args,
        typename = std::enable_if_t<not std::is_member_function_pointer<F>::value>, // ensure here that function object
        typename = std::enable_if_t<not std::is_member_object_pointer<F>::value>>   // types are correctly deduced
    inline constexpr decltype(auto) defer (F && f, Args&& ... args)                 // (prevents greedy template instantiation)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        static_assert (ftraits::arity == sizeof...(args), "arity of function disagrees with parameter list length");

        return
            [=, args = std::make_tuple(std::forward<Args>(args)...)] (void) 
            {
                return fnk::eval_tuple (f, std::move(args));
            };
    }

    // (const) member (const) functions
    template <class R, class C, typename ... Args>
    inline constexpr decltype(auto) defer (R(C::*f)() const, C const& c, Args&& ... args)
    {
        using ftraits = typename fnk::type_support::function_traits<decltype(f)>;
        static_assert (ftraits::arity == sizeof...(args), "arity of function disagrees with parameter list length");

        return
            [=, args = std::make_tuple(std::forward<Args>(args)...)] (void)
            {
                return fnk::eval_tuple (c.*f, std::move(args));
            };
    }

    template <class R, class C, typename ... Args>
    inline constexpr decltype(auto) defer (R(C::*f)() const, C & c, Args&& ... args)
    {
        using ftraits = typename fnk::type_support::function_traits<decltype(f)>;
        static_assert (ftraits::arity == sizeof...(args), "arity of function disagrees with parameter list length");

        return
            [=, args = std::make_tuple(std::forward<Args>(args)...)] (void)
            {
                return fnk::eval_tuple (c.*f, std::move(args));
            };
    }

    template <class R, class C, typename ... Args>
    inline constexpr decltype(auto) defer (R(C::*f)(), C const& c, Args&& ... args)
    {
        using ftraits = typename fnk::type_support::function_traits<decltype(f)>;
        static_assert (ftraits::arity == sizeof...(args), "arity of function disagrees with parameter list length");

        return
            [=, args = std::make_tuple(std::forward<Args>(args)...)] (void)
            {
                return fnk::eval_tuple (c.*f, std::move(args));
            };
    }

    template <class R, class C, typename ... Args>
    inline constexpr decltype(auto) defer (R(C::*f)(), C & c, Args&& ... args)
    {
        using ftraits = typename fnk::type_support::function_traits<decltype(f)>;
        static_assert (ftraits::arity == sizeof...(args), "arity of function disagrees with parameter list length");

        return
            [=, args = std::make_tuple(std::forward<Args>(args)...)] (void)
            {
                return fnk::eval_tuple (c.*f, std::move(args));
            };
    }

    // (const) member (const) objects
    template <class R, class C>
    inline constexpr decltype(auto) defer (R(C::*m), C && c)
    {
        return [r = fnk::eval (m, c)] (void) { return r; };
    }

    template <class R, class C>
    inline constexpr decltype(auto) defer (R(C::*m), C & c)
    {
        return [r = fnk::eval (m, c)] (void) { return r; };
    }

    // 
    // Tuple Versions
    //
    template <class F, typename Tup>
    constexpr decltype(auto) defer_tuple (F && f, Tup && t)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        using tupsize = typename std::tuple_size<typename std::decay<Tup>::type>;
        static_assert (ftraits::arity == tupsize::value, "arity of function disagrees with parameter tuple length");

        return
            [=] (void) 
            {
                return fnk::eval_tuple (f, std::forward<Tup>(t));
            };
    }
} // namespace fnk

#endif // ifndef DEFER_HPP

