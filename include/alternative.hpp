//
// Alternative types in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodrd@gmail.com
// License: Please see LICENSE.md
//

#ifndef ALTERNATIVE_HPP
#define ALTERNATIVE_HPP

#include <type_traits>

#include "applicative_functor.hpp"
#include "monoid.hpp"

namespace fnk
{
    template <typename T>
    struct alternative
    {
        struct is_alternative_instance : public std::false_type {};
    };

    //
    // Satisfies the following laws:
    //      (i) `alt` is associative
    //      (ii) `empty` is the identity for `alt`; that is, `alt` (`empty`, _) = `alt` (_, `empty`) = _
    //
    template <typename T,
        typename = std::enable_if_t<fnk::monoid<T>::is_monoid_instance::value>,
        typename = std::enable_if_t<fnk::applicative_functor<T>::is_applicative_functor_instance::value>>
    struct default_alternative : public alternative<T>
    {
        static inline constexpr decltype(auto) empty (void) { return fnk::monoid<T>::unity (); } 

        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_,T>::value>>
        static inline constexpr decltype(auto) alt (T_ && l, T_ && r)
        {
            return fnk::monoid<T>::append (std::forward<T_>(l), std::forward<T_>(r));
        }

        struct is_alternative_instance : public std::true_type {};
    };

    //
    // Utility infix operator to replate explicit `alt`
    //

    template <typename A,
        typename = std::enable_if_t<fnk::alternative<A>::is_alternative_instance::value>>
    inline constexpr decltype(auto) operator| (A && l, A && r)
    {
        return fnk::alternative<A>::alt (l, r);
    }

    template <typename A,
        typename = std::enable_if_t<fnk::alternative<A>::is_alternative_instance::value>>
    inline constexpr decltype(auto) some (A && a)
    {
        return fnk::functor<A>::fmap
            ([](auto && e, auto && l) { return l.push_front(e); },
             fnk::applicative_functor<A>::apply (a, many(a)));
    }
    
    template <typename A,
        typename = std::enable_if_t<fnk::alternative<A>::is_alternative_instance::value>>
    inline constexpr decltype(auto) many (A && a)
    {
        return fnk::alternative<A>::alt (some (a), fnk::applicative_functor<A>::pure (std::list<A>{}));
    }
} // namespace fnk

#endif // ifndef ALTERNATIVE_HPP

