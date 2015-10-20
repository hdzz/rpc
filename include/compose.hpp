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
        using FArg =
            typename type_support::function_traits<F>::
                template argument<0>::type;
        using GRet =
            typename type_support::function_traits<G>::return_type;

        static_assert (std::is_convertible<GRet, FArg>::value,
                      "infered return and argument types are non-convertible.");
        static_assert (type_support::function_traits<F>::arity == 1,
                      "attempt to compose non-unary function.");
        static_assert (type_support::function_traits<G>::arity == 1,
                      "attempt to compose non-unary function.");

        return [=](auto && a) { return eval(f, eval(g, a)); };
    }

    template <class F, class ... Gs,
             typename = std::enable_if_t<sizeof...(Gs) != 0>>
    inline constexpr decltype(auto) compose (F && f, Gs&& ... gs) 
    {
        return compose (f, compose(gs...));
    }
 
    template <class F>
    inline constexpr decltype(auto) compose (std::tuple<F> f) 
    {
        return std::get<0>(f);
    }
 
    template <class ... Fs>
    inline constexpr decltype(auto) compose (std::tuple<Fs...> fs) 
    {
        return compose (std::get<0>(fs), compose(fnk::utility::tuple_tail(fs)));
    }
 
    template <typename F, typename G>
    inline constexpr decltype(auto) operator& (F && f, G && g)
    {
        return compose (f, g);
    }

    template <class F>
    inline constexpr decltype(auto) iterate (F && f, std::size_t const n = 0)
    {
        using ftraits = typename type_support::function_traits<F>;
        using fret = typename ftraits::return_type;
        using farg = typename ftraits::template argument<0>::type;
        static_assert (ftraits::arity == 1,
                      "cannot compose function with arity not equal to 1 with "
                      "itself");
        static_assert (std::is_convertible<fret, farg>::value,
                      "function return type must be convertible to it's "
                      "argument type");
   
        return [=](auto&& a)
        {
            auto ret (f(a));
            for (std::size_t i = 1; i <= n; ++i)
                ret = f (ret);
            return ret;
        };
    }

    //
    // Iteratively apply f until the predicate fails, or
    // if n is not equal to 0 then until the predicate fails or
    // n iterations have occured
    //
    template <class F, class P>
    inline constexpr decltype(auto) iterate_while
        (F && f, P && predicate, std::size_t const n = 0)
    {
        using ftraits = typename type_support::function_traits<F>;
        using fret = typename ftraits::return_type;
        using farg = typename ftraits::template argument<0>::type;
        static_assert (ftraits::arity == 1,
                      "cannot compose function with arity not equal to 1 with "
                      "itself");
        static_assert (std::is_convertible<fret, farg>::value,
                      "function return type must be convertible to it's "
                      "argument type");
   
        return [=](auto&& a)
        {
            auto res (f(a));
            if (n != 0) {
                for (std::size_t i = 1; i <= n && predicate (res); ++i)
                    res = f (res);
            } else {
                while (predicate (res))
                    res = f (res);
            }
            return res;
        };
    }

    //
    // Composing N_i-Ary functions:
    //      compose(f, g_1,..., g_N)(a_1,..., a_N) = f(g_1(a_1),..., g_N(a_N))
    //
    template <typename F, typename ... Gs>
    inline constexpr decltype(auto) composeN (F && f, Gs&& ... gs)
    {
        using ftraits = typename type_support::function_traits<F>;
        static_assert (ftraits::arity == sizeof...(gs),
                      "arity does not match length of composed functions");

        return [=](auto && ... as)
        {
            static_assert (sizeof...(gs) == sizeof...(as),
                          "number of arguments does not match number of "
                          "functions to apply");
            return
                eval_tuple
                    (f,
                    utility::eval_tuple
                        (std::make_tuple(gs...),
                         std::make_tuple(std::forward(as)...),
                         typename utility::seq_gen<sizeof...(gs)>::type()));
        };
    }

    template <typename F, typename ... Gs>
    inline constexpr decltype(auto) composeN (F && f, std::tuple<Gs...> && gs)
    {
        using ftraits = typename type_support::function_traits<F>;
        using gtsize = std::tuple_size<decltype(gs)>;
        static_assert (ftraits::arity == gtsize::value,
                      "arity does not match length of composed functions");

        return [=](auto && ... as)
        {
            static_assert (gtsize::value == sizeof...(as),
                          "number of arguments does not match number of "
                          "functions to apply");
            return
                eval_tuple
                    (f,
                    utility::eval_tuple
                        (gs,
                         std::make_tuple(std::forward(as)...),
                         typename utility::seq_gen<gtsize::value>::type()));
        };
    }

    template <typename F, typename ... Gs>
    inline constexpr decltype(auto) operator& (F && f, std::tuple<Gs...> && gs)
    {
        return composeN (f, gs);
    }
} // namespace fnk

#endif // ifndef COMPOSE_HPP

