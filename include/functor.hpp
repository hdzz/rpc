//
// Functors in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard
// License: Please see LICENSE.md
//

#ifndef FUNCTOR_HPP
#define FUNCTOR_HPP

#include <type_traits>

#include "mappable.hpp"
#include "maybe.hpp"
#include "type_support/function_traits.hpp"

namespace fnk
{
    template <class T>
    struct functor;
    
    namespace detail
    {
        template <class T>
        struct not_functor
        {
            struct is_functor_instance : public std::false_type {};
        };
    } // namespace detail

    template <class T,
        typename = std::enable_if_t<fnk::mappable<T>::is_mappable_instance::value>>
    struct default_functor 
    {
        template <class F, typename U = typename fnk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<fnk::functor<U>::is_functor_instance::value>>
        static constexpr std::function<U(T)> fmap (F && f)
        {
            return [=] (T const& t)
            {
                return fnk::map (f, t);
            };
        }

        template <class F, typename T_, typename U = typename fnk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<fnk::functor<U>::is_functor_instance::value>,
            typename = std::enable_if_t<std::is_same<T, T_>::value>>
        static constexpr U fmap (F && f, T_ && t)
        {
            return fnk::map (f, std::forward<T_>(t));
        }

        struct is_functor_instance : public std::true_type {};
    };

    template <class T>
    struct functor : std::conditional_t
                        <fnk::mappable<T>::is_mappable_instance::value, default_functor<T>, detail::not_functor<T>> {};

    template <class T>
    struct functor<fnk::maybe<T>> 
    {
        template <class F>
        static constexpr decltype(auto) fmap (F && f)
        {
            using U = typename fnk::type_support::function_traits<F>::return_type;
            return [=] (fnk::maybe<T> const& m)
            {
                return static_cast<bool>(m) ? fnk::make_maybe (f(*m)) : fnk::maybe<U> {};
            };
        }
        template <class F>
        static constexpr decltype(auto) fmap (F && f, fnk::functor<fnk::maybe<T>> const& m)
        {
            using U = typename fnk::type_support::function_traits<F>::return_type;
            if (m)
              return fnk::make_maybe (f(*m));
            else
              return fnk::maybe<U> {}; 
        }

        struct is_functor_instance : public std::true_type {};
    };

    template <template <typename...> class C, typename ... Ts>
    struct functor<C<Ts...>> : public default_functor<C<Ts...>>
    {
        template <class F,
            typename = std::enable_if_t<fnk::utility::is_well_formed<F(Ts...)>::value>>
        static decltype(auto) fmap (F && f)
        {
            return [=] (C<Ts...> const& c)
            {
                return fnk::map (f, c);
            };
        }

        template <class F, 
            typename = std::enable_if_t<fnk::utility::is_well_formed<F(Ts...)>::value>>
        static decltype(auto) fmap (F && f, C<Ts...> const& c)
        {
            return fnk::map (f, c);
        }
    };
} // namespace fnk

#endif // ifndef FUNCTOR_HPP

