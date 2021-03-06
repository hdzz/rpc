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

#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <string>

#include "concat.hpp"
#include "functor.hpp"
#include "mappable.hpp"
#include "maybe.hpp"
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
    struct default_applicative_functor : public fnk::functor<T>
    {
        template <typename A>
        static inline constexpr decltype(auto) pure (A &&);

        template <class F, typename U,
            typename = std::enable_if_t<std::is_same<std::decay_t<F>, T>::value>,
            typename = std::enable_if_t<std::is_same<std::decay_t<U>, T>::value>>
        static inline constexpr decltype(auto) apply (F const&, U const&);
    
        struct is_applicative_functor_instance : public std::true_type {};
    };

    template <typename C,
        typename = std::enable_if_t<fnk::functor<C>::is_functor_instance::value>>
    struct default_applicative_functor_container : public fnk::default_applicative_functor<C>
    {
        template <typename A>
        static inline constexpr decltype(auto) pure (A && a)
        {
            using OT = typename fnk::type_support::container_traits<C>::template rebind <A>;
            return OT {std::forward<A>(a)};
        }

        template <class F, typename U,
            typename = std::enable_if_t<std::is_same<std::decay_t<F>, C>::value>,
            typename = std::enable_if_t<std::is_same<std::decay_t<U>, C>::value>>
        static inline constexpr decltype(auto) apply (F const& fs, U const& us)
        {
            using FT = typename fnk::type_support::container_traits<F>::value_type;
            using OT = typename fnk::type_support::container_traits<C>::template 
                rebind<typename fnk::type_support::function_traits<FT>::return_type>;
            OT out {};
            for (auto const& f : fs)
               for (auto const& u : us)
                  fnk::type_support::container_traits<OT>::insert (out, fnk::eval (f, u));
           return out; 
        }
    };

#define DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(C)\
        template <class ... Args>\
        struct applicative_functor<C<Args...>> : public default_applicative_functor_container<C<Args...>> {};\
        template <class ... Args>\
        struct applicative_functor<C<Args...> &> : public default_applicative_functor_container<C<Args...>> {};\
        template <class ... Args>\
        struct applicative_functor<C<Args...> const> : public default_applicative_functor_container<C<Args...>> {};\
        template <class ... Args>\
        struct applicative_functor<C<Args...> const&> : public default_applicative_functor_container<C<Args...>> {};\
        template <class ... Args>\
        struct applicative_functor<C<Args...> &&> : public default_applicative_functor_container<C<Args...>> {};

    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::deque);
    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::forward_list);
    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::list);
    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::set);
    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::multiset);
    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::unordered_set);
    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::unordered_multiset);
    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::basic_string);
    DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE(std::vector);

#undef DEFAULT_APPLICATIVE_FUNCTOR_CONTAINER_INSTANCE

    template <typename T>
    struct applicative_functor<fnk::maybe<T>> : public default_applicative_functor<fnk::maybe<T>>
    {
        template <typename A,
            typename = std::enable_if_t<std::is_convertible<A,T>::value>>
        static inline constexpr decltype(auto) pure (A && a)
        {
            return fnk::make_maybe<T> (std::forward<A>(a));
        }

        template <class F, typename U,
            typename = std::enable_if_t<std::is_same<std::decay_t<U>, fnk::maybe<T>>::value>>
        static inline constexpr decltype(auto) apply (F && f, U && u)
        {
            using A = typename std::decay_t<U>::value_type; // U is maybe in this case
            using B = typename std::decay_t<F>::value_type; // F is maybe in this case
            return static_cast<bool>(std::forward<F>(f))
                ? fnk::functor<fnk::maybe<A>>::fmap (*f, std::forward<U>(u))
                : fnk::maybe<typename fnk::type_support::function_traits<B>::return_type>();
        }
    };

    template <typename T>
    struct applicative_functor<fnk::maybe<T> const> : applicative_functor<fnk::maybe<T>> {};

    template <typename T>
    struct applicative_functor<fnk::maybe<T> &> : applicative_functor<fnk::maybe<T>> {};

    template <typename T>
    struct applicative_functor<fnk::maybe<T> const&> : applicative_functor<fnk::maybe<T>> {};

    template <typename T>
    struct applicative_functor<fnk::maybe<T> &&> : applicative_functor<fnk::maybe<T>> {};

    //
    // Utility function
    //
    template <class F, class T>
    inline constexpr decltype(auto) apply (F && f, T && t)
    {
        return fnk::applicative_functor<T>::apply (f, t);
    }
/*
    template <class F, class G,
        typename = std::enable_if_t<fnk::applicative_functor<F>::is_applicative_functor_instance::value>,
        typename = std::enable_if_t<fnk::applicative_functor<G>::is_applicative_functor_instance::value>>
    inline decltype(auto) insertapply (F && f, G && g)
    {
        using FR = typename fnk::type_support::function_traits<F>::return_type;
        using V  = typename fnk::type_support::container_traits<FR>::value_type;
        return fnk::functor<FR>::fmap (fnk::type_support::insert<FR, V>, apply (f, g));
    }
    
    template <class F, class G,
        typename = std::enable_if_t<fnk::applicative_functor<F>::is_applicative_functor_instance::value>,
        typename = std::enable_if_t<fnk::applicative_functor<G>::is_applicative_functor_instance::value>>
    inline decltype(auto) concatapply (F && f, G && g)
    {
        using FR = typename fnk::type_support::function_traits<F>::return_type;
        return fnk::functor<FR>::fmap (fnk::concat<FR>, apply (f, g));
    }
*/
} // namespace fnk

#endif // ifndef APPLICATIVE_FUNCTOR_HPP

