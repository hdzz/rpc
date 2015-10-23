//
// Some utility code for manipulation of std::tuple types
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef TUPLE_UTILS_HPP
#define TUPLE_UTILS_HPP

#include "type_utils.hpp"
#include "../type_support/function_traits.hpp"

namespace fnk
{
namespace utility
{
    template <class F, typename Tup, std::size_t ... S>
    static inline constexpr decltype(auto) call (F &&, Tup &&, fnk::utility::seq<S...>);

    template <class F, typename Tup, std::size_t ... S>
    static inline constexpr decltype(auto) bind (F &&, Tup &&, fnk::utility::seq<S...>);

    template <template <typename ...> class FTup, template <typename ...> class ATup,
        class ... Fs, typename ... As, std::size_t ... S>
    static inline constexpr decltype(auto) eval_tuple (FTup<Fs...> &&, ATup<As...> &&, fnk::utility::seq<S...>);

    template <std::size_t I>
    struct tuple_helper
    {
        template <typename ... Args>
        using elem_t = std::remove_reference_t<decltype(std::get<I>(std::declval<std::tuple<Args...>>()))>;

        template <typename ... Args>
        static inline constexpr bool any_equal (std::tuple<Args...> const& l, std::tuple<Args...> const& r)
        {
            if (std::get<I>(l) == std::get<I>(r))
                return true;
            return tuple_helper<I-1>::any_equal(l,r);
        }

        template <typename ... Args>
        static inline void increment (std::tuple<Args...> & a)
        {
            tuple_helper<I-1>::increment(a);
            ++std::get<I>(a);
        }

        template <typename ... Args>
        static inline void increment (std::tuple<Args...> && a)
        {
            tuple_helper<I-1>::increment(std::forward(a));
            ++std::get<I>(std::forward(a));
        }

        template <class Tup, typename ... Args>
        static inline constexpr decltype(auto) dereference (Tup && t, Args&& ... as)
        {
            return tuple_helper<I-1>::dereference
                (std::forward<Tup>(t), std::get<I>(std::forward<Tup>(t)), std::forward<Args>(as)...);
        }

        template <typename ... Args, class F>
        static inline constexpr void for_each (std::tuple<Args...> & t, F && f)
        {
            f (std::get<I>(t));
            tuple_helper<I-1>::for_each (t, f);
        }

        template <typename ... Args, class F>
        static inline constexpr void for_each (std::tuple<Args...> const& t, F && f)
        {
            f (std::get<I>(t));
            tuple_helper<I-1>::for_each (t, f);
        }

        template <typename ... Args, class F>
        static inline constexpr void for_each_index (std::tuple<Args...> & t, F && f)
        {
            f (std::get<I>(t), I);
            tuple_helper<I-1>::for_each_index (t, f);
        }

        template <typename ... Args, class F>
        static inline constexpr void for_each_index (std::tuple<Args...> const& t, F && f)
        {
            f (std::get<I>(t), I);
            tuple_helper<I-1>::for_each_index (t, f);
        }
    };

    template <>
    struct tuple_helper<0>
    {
        template <typename ... Args>
        using elem_t = std::remove_reference_t<decltype(std::get<0>(std::declval<std::tuple<Args...>>()))>;

        template <typename ... Args>
        static inline constexpr bool any_equal (std::tuple<Args...> const& l, std::tuple<Args...> const& r)
        {
            return std::get<0>(l) == std::get<0>(r); 
        }

        template <typename ... Args>
        static inline void increment (std::tuple<Args...> & a)
        {
            ++std::get<0>(a);
        }

        template <typename ... Args>
        static inline void increment (std::tuple<Args...> && a)
        {
            ++std::get<0>(std::forward(a));
        }

        template <class Tup, typename ... Args>
        static inline constexpr decltype(auto) dereference (Tup && t, Args&& ... as)
        {
            return std::tuple<decltype(*std::get<0>(t)), decltype(*std::forward<Args>(as))...>
                (*std::get<0>(t), (*std::forward<Args>(as))...);
        }

        template <typename ... Args, class F>
        static inline constexpr void for_each (std::tuple<Args...> & t, F && f)
        {
            f (std::get<0>(t));
        }

        template <typename ... Args, class F>
        static inline constexpr void for_each (std::tuple<Args...> const& t, F && f)
        {
            f (std::get<0>(t));
        }

        template <typename ... Args, class F>
        static inline constexpr void for_each_index (std::tuple<Args...> & t, F && f)
        {
            f (std::get<0>(t), 0);
        }

        template <typename ... Args, class F>
        static inline constexpr void for_each_index (std::tuple<Args...> const& t, F && f)
        {
            f (std::get<0>(t), 0);
        }
    };

    template <class F, typename Tup, std::size_t ... S>
    static inline constexpr decltype(auto) call (F && f, Tup && argt, fnk::utility::seq<S...>)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        using tsize = std::tuple_size<typename std::decay_t<Tup>>;
        static_assert (ftraits::arity == tsize::value, "arity of function does not agree with parameter tuple length");

        return f(std::get<S>(std::forward<Tup>(argt))...);
    }

    template <class F, typename Tup, std::size_t ... S>
    static inline constexpr decltype(auto) bind (F && f, Tup && t, fnk::utility::seq<S...>)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        using tsize = std::tuple_size<typename std::decay_t<Tup>>;
        static_assert (ftraits::arity == tsize::value, "arity of function does not agree with parameter tuple length");

        return std::bind (f, std::get<S>(std::forward<Tup>(t))...);
    }

    template <template <typename ...> class FTup, template <typename ...> class ATup,
        class ... Fs, typename ... As, std::size_t ... S>
    static inline constexpr decltype(auto) eval_tuple (FTup<Fs...> && fs, ATup<As...> && as, fnk::utility::seq<S...>)
    {
        using ftsize = std::tuple_size<FTup<Fs...>>;
        using atsize = std::tuple_size<ATup<As...>>;
        static_assert (ftsize::value == atsize::value,
                "cannot component-wise apply function tuple to argument tuple of different length");

        return std::make_tuple
            (fnk::utility::call
                (std::get<S>(fs),
                 std::get<S>(as), 
                 typename fnk::utility::seq_gen<
                    std::tuple_size<typename std::tuple_element_t<S, typename std::decay_t<decltype(as)>>>::value>::type())...);
    }

    template <class A, class ... As>
    static inline constexpr bool tuple_any_equal (std::tuple<A,As...> const& l, std::tuple<A,As...> const& r)
    {
        return tuple_helper<sizeof...(As)>::any_equal (l, r);
    }

    template <class A, class ... As>
    static inline void tuple_increment (std::tuple<A,As...> & a)
    {
        tuple_helper<sizeof...(As)>::increment (a);
    }

    template <class A, class ... As>
    static inline void tuple_increment (std::tuple<A,As...> && a)
    {
        tuple_helper<sizeof...(As)>::increment (std::forward(a));
    }

namespace detail
{
    template <typename Tup, typename Seq>
    struct tuple_tail_helper {};

    template <typename Tup, std::size_t S0, std::size_t... Seq>
    struct tuple_tail_helper<Tup, seq<S0, Seq...>>
    {
        using type = std::tuple<typename std::tuple_element<Seq, Tup>::type...>;
    };

    template <typename Tup>
    struct tuple_tail
        : tuple_tail_helper<Tup, seq_gen<std::tuple_size<Tup>::value>>
    {};

    template <typename Tup, std::size_t S0, std::size_t ... Seq>
    static constexpr inline decltype(auto)
    tuple_tailfn (Tup && t, seq<S0, Seq...>)
    {
        return std::make_tuple (std::get<Seq>(std::forward<Tup>(t))...); 
    }
} // namespace detail

    template <typename Tup>
    static constexpr inline decltype(auto) tuple_tail (Tup && t)
    {
        return detail::tuple_tailfn
            (std::forward<Tup>(t),
             typename seq_gen<std::tuple_size<Tup>::value>::type{}); 
    }

    template <class Tup>
    static inline constexpr decltype(auto) tuple_dereference (Tup && t)
    {
        return tuple_helper<std::tuple_size<std::decay_t<Tup>>::value-1>::dereference (std::forward<Tup>(t));
    }

    template <typename ... Args, class F>
    static inline constexpr void tuple_for_each (std::tuple<Args...> & t, F && f)
    {
        tuple_helper<std::tuple_size<decltype(t)>::value-1>::for_each (t, f); 
    }

    template <typename ... Args, class F>
    static inline constexpr void tuple_for_each_index (std::tuple<Args...> & t, F && f)
    {
        tuple_helper<std::tuple_size<decltype(t)>::value-1>::for_each_index (t, f); 
    }
} // namespace utility
} // naemspace fnk

#endif // ifndef TUPLE_UTILS_HPP

