//
// Compisition of functions
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef COMPOSE_HPP
#define COMPOSE_HPP

#include <functional>
#include <type_traits>

#include "defer.hpp"
#include "eval.hpp"
#include "type_support/function_traits.hpp"

namespace fnk
{
    //
    // Composing 1-Ary functions
    //      compose(f, g)(a) = (f & g)(a) = f(g(a))
    //
    template <class F, class G>
    inline constexpr decltype(auto) compose (F && f, G && g) 
    {
        using FArg = typename fnk::type_support::function_traits<F>::template argument<0>::type;
        using GRet = typename fnk::type_support::function_traits<G>::return_type;

        static_assert (std::is_convertible<GRet, FArg>::value, "types are non-convertible.");
        static_assert (fnk::type_support::function_traits<F>::arity == 1, "attempt to compose non-unary function.");
        static_assert (fnk::type_support::function_traits<G>::arity == 1, "attempt to compose non-unary function.");

        return [=](auto && a) { return fnk::eval(f, fnk::eval(g, a)); };
    }

    template <class F, class ... Gs>
    inline constexpr decltype(auto) compose (F && f, Gs&& ... gs) 
    {
        return compose (f, compose(gs...));
    }

    template <typename F, typename G>
    inline constexpr decltype(auto) operator& (F && f, G && g)
    {
        return compose (f, g);
    }

    //
    // Composing N_i-Ary functions:
    //      compose(f, g_1, ..., g_N)(a_1, ..., a_N) = f(g_1(a_1), ..., g_N(a_N))
    //
    template <typename F, typename ... Gs>
    inline constexpr decltype(auto) composeN (F && f, Gs&& ... gs)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        static_assert (ftraits::arity == sizeof...(gs), "arity does not match length of composed functions");

        return [=](auto && ... as)
        {
            static_assert (sizeof...(gs) == sizeof...(as), "number of arguments does not match number of functions to apply");
            return fnk::eval_tuple
                (f,
                 fnk::utility::eval_tuple
                        (std::make_tuple(gs...),
                         std::make_tuple(std::forward(as)...),
                         typename fnk::utility::seq_gen<sizeof...(gs)>::type()));
        };
    }

    template <typename F, typename ... Gs>
    inline constexpr decltype(auto) composeN (F && f, std::tuple<Gs...> && gs)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        using gtsize = std::tuple_size<decltype(gs)>;
        static_assert (ftraits::arity == gtsize::value, "arity does not match length of composed functions");

        return [=](auto && ... as)
        {
            static_assert (gtsize::value == sizeof...(as), "number of arguments does not match number of functions to apply");
            return fnk::eval_tuple
                (f,
                 fnk::utility::eval_tuple
                    (gs,
                     std::make_tuple(std::forward(as)...),
                     typename fnk::utility::seq_gen<gtsize::value>::type()));
        };
    }

    template <typename F, typename ... Gs>
    inline constexpr decltype(auto) operator& (F && f, std::tuple<Gs...> && gs)
    {
        return composeN (f, gs);
    }
} // namespace fnk

#endif // ifndef COMPOSE_HPP

