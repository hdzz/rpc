//
// Applicative Functors in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard
// License: Please see LICENSE.md
//

#ifndef APPLICATIVE_FUNCTOR_HPP
#define APPLICATIVE_FUNCTOR_HPP

#include <type_traits>

#include "functor.hpp"
#include "mappable.hpp"
#include "type_support/maybe.hpp"
#include "type_support/function_traits.hpp"

namespace fnk
{
    template <typename T>
    struct applicative_functor
    {
        struct is_applicative_functor_instance : public std::false_type {};   
    };

    template <typename T,
        typename = std::enable_if_t<fnk::functor<T>::is_functor_instance::value>>
    struct default_applicative_functor : public functor<T>
    {
        template <typename T_,
            std::enable_if_t<std::is_convertible<T_,T>::value>>
        static inline constexpr decltype(auto) pure (T_ &&);

        template <class F, typename U,
            typename = std::enable_if_t<std::is_same<U, T>::value>>
        static inline constexpr decltype(auto) apply (F &&, U &&);
    
        struct is_applicative_functor_instance : public std::true_type {};
    };

    template <typename T>
    struct applicative_functor<fnk::type_support::maybe<T>> : public default_applicative_functor<fnk::type_support::maybe<T>>
    {
        template <typename T_,
            typename = std::enable_if_t<std::is_convertible<T_,T>::value>>
        static inline constexpr decltype(auto) pure (T_ && t)
        {
            return fnk::type_support::make_maybe (std::forward<T>(t));
        }

        template <class F, typename U,
            typename = std::enable_if_t<std::is_same<std::decay_t<U>, fnk::type_support::maybe<T>>::value>>
        static inline constexpr decltype(auto) apply (F && f, U && u)
        {
            using A = typename std::decay_t<U>::value_type; // U is maybe in this case
            using B = typename std::decay_t<F>::value_type; // F is maybe in this case
            return static_cast<bool>(std::forward<F>(f))
                ? fnk::functor<fnk::type_support::maybe<A>>::fmap (*f, std::forward<U>(u))
                : fnk::type_support::maybe<typename fnk::type_support::function_traits<B>::return_type>();
        }
    };

    template <typename T>
    struct applicative_functor<fnk::type_support::maybe<T> const> : applicative_functor<fnk::type_support::maybe<T>> {};

    template <typename T>
    struct applicative_functor<fnk::type_support::maybe<T> &> : applicative_functor<fnk::type_support::maybe<T>> {};

    template <typename T>
    struct applicative_functor<fnk::type_support::maybe<T> const&> : applicative_functor<fnk::type_support::maybe<T>> {};

    template <typename T>
    struct applicative_functor<fnk::type_support::maybe<T> &&> : applicative_functor<fnk::type_support::maybe<T>> {};

    //
    // Utility function
    //
    template <class F, class T>
    inline constexpr decltype(auto) apply (F && f, T && t)
    {
        return fnk::applicative_functor<T>::apply (f, t);
    }
} // namespace fnk

#endif // ifndef APPLICATIVE_FUNCTOR_HPP

