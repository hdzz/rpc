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

namespace funk
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
        typename = std::enable_if_t<funk::mappable<T>::is_mappable_instance::value>>
    struct default_functor 
    {
        template <class F, typename U = typename funk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<funk::functor<U>::is_functor_instance::value>>
        static constexpr std::function<U(T)> fmap (F && f)
        {
            return [=] (T const& t)
            {
                return funk::map (f, t);
            };
        }

        template <class F, typename T_, typename U = typename funk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<funk::functor<U>::is_functor_instance::value>,
            typename = std::enable_if_t<std::is_same<T, T_>::value>>
        static constexpr U fmap (F && f, T_ && t)
        {
            return funk::map (f, std::forward<T_>(t));
        }

        struct is_functor_instance : public std::true_type {};
    };

    template <class T>
    struct functor : std::conditional_t
                        <funk::mappable<T>::is_mappable_instance::value, default_functor<T>, detail::not_functor<T>> {};

    template <class T>
    struct functor<funk::maybe<T>> 
    {
        template <class F>
        static constexpr decltype(auto) fmap (F && f)
        {
            using U = typename funk::type_support::function_traits<F>::return_type;
            return [=] (funk::maybe<T> const& m)
            {
                return static_cast<bool>(m) ? funk::make_maybe (f(*m)) : funk::maybe<U> {};
            };
        }
        template <class F>
        static constexpr decltype(auto) fmap (F && f, funk::functor<funk::maybe<T>> const& m)
        {
            using U = typename funk::type_support::function_traits<F>::return_type;
            if (m)
              return funk::make_maybe (f(*m));
            else
              return funk::maybe<U> {}; 
        }

        struct is_functor_instance : public std::true_type {};
    };

    template <template <typename...> class C, typename ... Ts>
    struct functor<C<Ts...>> : public default_functor<C<Ts...>>
    {
        template <class F,
            typename = std::enable_if_t<funk::utility::is_well_formed<F(Ts...)>::value>>
        static decltype(auto) fmap (F && f)
        {
            return [=] (C<Ts...> const& c)
            {
                return funk::map (f, c);
            };
        }

        template <class F, 
            typename = std::enable_if_t<funk::utility::is_well_formed<F(Ts...)>::value>>
        static decltype(auto) fmap (F && f, C<Ts...> const& c)
        {
            return funk::map (f, c);
        }
    };
} // namespace funk

#endif // ifndef FUNCTOR_HPP

