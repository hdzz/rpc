//
// Generalized (immediate) function evaluation.
//
// Author: Dalton Wodoard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef EVAL_HPP
#define EVAL_HPP

#include <tuple>
#include <type_traits>

#include "algebraic.hpp"
#include "type_support/function_traits.hpp"
#include "utility/tuple_utils.hpp"

namespace fnk
{
    // function objects:
    // ensure here that function object types are correctly
    // deduced to prevent greedy template instantiaion.
    template <class F, typename ... Args,
        typename = std::enable_if_t
            <not std::is_member_function_pointer<F>::value>,
        typename = std::enable_if_t
            <not std::is_member_object_pointer<F>::value>>
    inline constexpr decltype(auto) eval (F && f, Args&& ... args)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        static_assert (ftraits::arity == sizeof...(args),
                      "arity of function disagrees with parameter list length");

        return f (std::forward<Args>(args)...);
    }

    // (const) member (const) functions
    template <class R, class C, typename ... Args>
    inline constexpr decltype(auto) eval
        (R(C::*f)() const, C const& c, Args&& ... args)
    {
        using ftraits =
            typename fnk::type_support::function_traits<decltype(f)>;
   
        // "+ 1" accounts for implicit "this" argument
        static_assert (ftraits::arity == 1 + sizeof...(args),
                      "arity of function disagrees with parameter list length");

        return (c.*f) (std::forward<Args>(args)...);
    }

    template <class R, class C, typename ... Args>
    inline constexpr decltype(auto) eval
        (R(C::*f)() const, C & c, Args&& ... args)
    {
        using ftraits =
            typename fnk::type_support::function_traits<decltype(f)>;

        // "+ 1" accounts for implicit "this" argument
        static_assert (ftraits::arity == 1 + sizeof...(args),
                      "arity of function disagrees with parameter list length");

        return (c.*f) (std::forward<Args>(args)...);
    }

    template <class R, class C, typename ... Args>
    inline constexpr decltype(auto) eval
        (R(C::*f)(), C const& c, Args&& ... args)
    {
        using ftraits =
            typename fnk::type_support::function_traits<decltype(f)>;

        // "+ 1" accounts for implicit "this" argument
        static_assert (ftraits::arity == 1 + sizeof...(args),
                      "arity of function disagrees with parameter list length");

        return (c.*f) (std::forward<Args>(args)...);
    }

    template <class R, class C, typename ... Args>
    inline constexpr decltype(auto) eval (R(C::*f)(), C & c, Args&& ... args)
    {
        using ftraits =
            typename fnk::type_support::function_traits<decltype(f)>;

        // "+ 1" accounts for implicit "this" argument
        static_assert (ftraits::arity == 1 + sizeof...(args),
                      "arity of function disagrees with parameter list length");

        return (c.*f) (std::forward<Args>(args)...);
    }

    // (const) member (const) objects
    template <class R, class C>
    inline constexpr decltype(auto) eval (R(C::*m), C const& c) { return c.*m; }

    template <class R, class C>
    inline constexpr decltype(auto) eval (R(C::*m), C & c) { return c.*m; }

    //
    // Tuple versions
    //
    template <class F, typename Tup,
        typename = std::enable_if_t
            <not std::is_member_function_pointer<F>::value>,
        typename = std::enable_if_t
            <not std::is_member_object_pointer<F>::value>>
    inline constexpr decltype(auto) eval_tuple (F && f, Tup && t)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        using tupsize =
            typename std::tuple_size<typename std::decay<Tup>::type>;
        static_assert (ftraits::arity == tupsize::value,
                      "arity of function disagrees with parameters length");

        return fnk::utility::call
            (f,
             std::forward<Tup>(t),
             typename fnk::utility::seq_gen<tupsize::value>::type());
    }

    template <class F, typename ... Ts>
    inline constexpr decltype(auto) eval_adt (F && f, fnk::adt<Ts...> const& a)
    {
        using ftraits = typename fnk::type_support::function_traits<F>;
        using U = typename ftraits::template argument<0>::type;
        
        static_assert 
            (fnk::adt<Ts...>::template is_adt_type<U>::value,
             "function cannot be applied to algebraic data type");

        return eval (f, a.template value<U>()); 
    }
} // namespace fnk

#endif // ifndef EVAL_HPP
