//
// Implementation of monads in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef MONAD_HPP
#define MONAD_HPP

#include <type_traits>

#include <deque>
#include <string>
#include <list>
#include <set>
#include <vector>

#include "applicative_functor.hpp"
#include "concat.hpp"
#include "functor.hpp"
#include "mappable.hpp"

namespace fnk
{
    template <typename T>
    struct monad
    {
        struct is_monad_instance : public std::false_type {};
    };

    template <typename T,
        typename = std::enable_if_t<fnk::applicative_functor<T>::is_applicative_functor_instance::value>>
    struct default_monad : public fnk::applicative_functor<T>
    {
        template <typename A>
        static inline constexpr decltype(auto) mreturn (A && a)
        {
            fnk::applicative_functor<T>::pure (std::forward<A>(a));
        }

        template <class F, typename T_,
            typename = std::enable_if_t<std::is_same<std::decay_t<T_>, T>::value>>
        static inline constexpr decltype(auto) mbind (T_ &&, F &&);

        template <class F>
        static inline constexpr decltype(auto) mlift (F && f)
        {
            return fnk::functor<T>::fmap (f);
        }

        struct is_monad_instance : public std::true_type {};
    };

    template <typename C,
        typename = std::enable_if_t<fnk::is_concatable_instance<C>::value>,
        typename = std::enable_if_t<fnk::applicative_functor<C>::is_applicative_functor_instance::value>>
    struct default_monad_container : public default_monad<C>
    {
        template <class F, typename C_,
            typename = std::enable_if_t<std::is_same<std::decay_t<C_>, C>::value>>
        static inline constexpr decltype(auto) mbind (C_ && c, F && f)
        {
            return fnk::concat (fnk::map (f, c)); 
        }
    };
 
#define DEFAULT_MONAD_CONTAINER_INSTANCE(C)\
        template <class ... Args>\
        struct monad<C<Args...>> : public default_monad_container<C<Args...>> {};\
        template <class ... Args>\
        struct monad<C<Args...> &> : public default_monad_container<C<Args...>> {};\
        template <class ... Args>\
        struct monad<C<Args...> const> : public default_monad_container<C<Args...>> {};\
        template <class ... Args>\
        struct monad<C<Args...> const&> : public default_monad_container<C<Args...>> {};\
        template <class ... Args>\
        struct monad<C<Args...> &&> : public default_monad_container<C<Args...>> {};

    DEFAULT_MONAD_CONTAINER_INSTANCE(std::deque);
    DEFAULT_MONAD_CONTAINER_INSTANCE(std::list);
    DEFAULT_MONAD_CONTAINER_INSTANCE(std::multiset);
    DEFAULT_MONAD_CONTAINER_INSTANCE(std::set);
    DEFAULT_MONAD_CONTAINER_INSTANCE(std::basic_string);
    DEFAULT_MONAD_CONTAINER_INSTANCE(std::vector);

#undef DEFAULT_MONAD_CONTAINER_INSTANCE

    template <typename T>
    struct monad<fnk::maybe<T>> : public default_monad<fnk::maybe<T>>
    {
        template <class F, typename A,
            typename = std::enable_if_t<std::is_same<std::decay_t<A>, T>::value>>
        static inline constexpr decltype(auto) mbind (fnk::maybe<A> && m, F && f)
        {
            using B = typename fnk::type_support::function_traits<F>::return_type; 
            return static_cast<bool>(m) ? fnk::eval (f, *m) : fnk::maybe<B>();
        }
    };

    //
    // Utility functions
    //
    template <class F, typename M,
        typename = std::enable_if_t<fnk::monad<M>::is_monad_instance::value>>
    inline constexpr decltype(auto) operator>>= (M && m, F && f)
    {
        return fnk::monad<M>::mbind (m, f);
    }

    template <class F, class G>
    inline constexpr decltype(auto) mcompose (F && f, G && g)
    {
        return [=](auto && a) { return fnk::eval (g, a) >>= f; };
    } 
} // namespace fnk

#endif // MONAD_HPP

