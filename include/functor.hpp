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
    struct functor
    {
        struct is_functor_instance : public std::false_type {};   
    };
 
    template <typename T>
    struct default_functor 
    {
        template <class F, typename U = typename fnk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<fnk::functor<U>::is_functor_instance::value>>
        static constexpr std::function<U(T)> fmap (F &&);

        template <class F, typename T_, typename U = typename fnk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<fnk::functor<U>::is_functor_instance::value>,
            typename = std::enable_if_t<std::is_same<T, T_>::value>>
        static constexpr U fmap (F &&, T_ &&);

        struct is_functor_instance : public std::true_type {};
    };   

    template <class C,
        typename = std::enable_if_t<fnk::mappable<C>::is_mappable_instance::value>>
    struct default_functor_container : public default_functor<C>
    {
        template <class F, typename U = typename fnk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<fnk::functor<U>::is_functor_instance::value>>
        static constexpr std::function<U(C)> fmap (F && f)
        {
            return [=] (C const& c)
            {
                return fnk::map (f, c);
            };
        }

        template <class F, typename C_, typename U = typename fnk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<fnk::functor<U>::is_functor_instance::value>,
            typename = std::enable_if_t<std::is_same<std::decay_t<C_>, C>::value>>
        static constexpr U fmap (F && f, C_ && c)
        {
            return fnk::map (f, std::forward<C_>(c));
        }

        struct is_functor_instance : public std::true_type {};
    };

#define DEFAULT_FUNCTOR_CONTAINER_INSTANCE(C)\
        template <class ... Args>\
        struct functor<C<Args...>> : public default_functor_container<C<Args...>> {};\
        template <class ... Args>\
        struct functor<C<Args...> &> : public default_functor_container<C<Args...>> {};\
        template <class ... Args>\
        struct functor<C<Args...> const> : public default_functor_container<C<Args...>> {};\
        template <class ... Args>\
        struct functor<C<Args...> const&> : public default_functor_container<C<Args...>> {};\
        template <class ... Args>\
        struct functor<C<Args...> &&> : public default_functor_container<C<Args...>> {};

    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::deque);
    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::forward_list);
    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::list);
    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::set);
    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::multiset);
    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::unordered_set);
    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::unordered_multiset);
    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::basic_string);
    DEFAULT_FUNCTOR_CONTAINER_INSTANCE(std::vector);

#undef DEFAULT_FUNCTOR_CONTAINER_INSTANCE


    template <class T>
    struct functor<fnk::maybe<T>> : fnk::default_functor<fnk::maybe<T>>
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
        static constexpr decltype(auto) fmap (F && f, fnk::maybe<T> const& m)
        {
            using U = typename fnk::type_support::function_traits<F>::return_type;
            return static_cast<bool>(m) ? fnk::make_maybe (f(*m)) : fnk::maybe<U> {}; 
        }
    };

    template <class T>
    struct functor<fnk::maybe<T> const> : public functor<fnk::maybe<T>> {};
    
    template <class T>
    struct functor<fnk::maybe<T> &> : public functor<fnk::maybe<T>> {};
    
    template <class T>
    struct functor<fnk::maybe<T> const&> : public functor<fnk::maybe<T>> {};
    
    template <class T>
    struct functor<fnk::maybe<T> &&> : public functor<fnk::maybe<T>> {};

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

    template <template <typename...> class C, typename ... Ts>
    struct functor<C<Ts...> const> : public functor<C<Ts...>> {};

    template <template <typename...> class C, typename ... Ts>
    struct functor<C<Ts...> &> : public functor<C<Ts...>> {};

    template <template <typename...> class C, typename ... Ts>
    struct functor<C<Ts...> const&> : public functor<C<Ts...>> {};

    template <template <typename...> class C, typename ... Ts>
    struct functor<C<Ts...> &&> : public functor<C<Ts...>> {};
} // namespace fnk

#endif // ifndef FUNCTOR_HPP

