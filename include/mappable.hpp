//
// Map a function to a container
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
// 

#ifndef MAPPABLE_HPP
#define MAPPABLE_HPP

#include <type_traits>
#include <functional>

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "defer.hpp"
#include "eval.hpp"
#include "type_support/container_traits.hpp"
#include "type_support/function_traits.hpp"
#include "utility/type_utils.hpp"

namespace funk
{
    template <class T>
    struct mappable
    {
        struct is_mappable_instance : public std::false_type {};
    };

    template <typename R, typename ... Args>
    struct mappable <R(Args...)>
    {
        typedef R value_type;

        template <class F, typename G,
            typename = std::enable_if_t
                <std::is_convertible
                    <typename funk::type_support::function_traits<F>::return_type, R>::value>,
            typename = std::enable_if_t<funk::mappable<G>::is_mappable_instance::value>,
            typename = std::enable_if_t<funk::utility::is_well_formed<G(Args...)>::value>>
        static constexpr decltype(auto) map (F && f, G && g)
        {
            return [=] (Args ... args)
            {
                return funk::eval (f, funk::eval(g, std::forward<Args>(args)...));
            }; 
        }

        struct is_mappable_instance : public std::true_type {};
    };

    template <typename R, class C, typename ... Args>
    struct mappable <R(C::*)(Args...) const> : public mappable<R(C const&, Args...)> {};

    template <typename R, class C, typename ... Args>
    struct mappable <R(C::*)(Args...)> : public mappable<R(C &, Args...)> {};

    template <typename R, class C>
    struct mappable <R(C::*)> : public mappable<R(C const&)> {};

    template <typename R, typename ... Args>
    struct mappable <R(*)(Args...)> : public mappable<R(Args...)> {};

    template <typename R, typename ... Args>
    struct mappable <R(&)(Args...)> : public mappable<R(Args...)> {};
    
    template <class C>
    struct default_mappable_container
    {
        typedef typename funk::type_support::container_traits<C>::value_type value_type;

        template <class F, class C_,
            typename = std::enable_if_t<std::is_convertible<C_, C>::value>,
            typename = std::enable_if_t
                <funk::utility::is_well_formed<F(typename funk::type_support::container_traits<C_>::value_type)>::value>>
        static decltype(auto) map (F && f, C_ && c)
        {
            using FR = typename funk::type_support::function_traits<F>::return_type;
            using U = funk::utility::rebind_argument_t<C_, C>;
            using OT = typename funk::type_support::container_traits<U>::template rebind<FR>;

            OT out;
            for (auto&& e : std::forward<U>(c))
                funk::type_support::container_traits<OT>::insert (out, funk::eval (f, e));
            return out;
        }

        struct is_mappable_instance : public std::true_type {};
    };

#define DEFAULT_MAPPABLE_CONTAINER_INSTANCE(C)\
    template <class ... Args>\
    struct mappable<C<Args...>> : public default_mappable_container<C<Args...>> {};\
    template <class ... Args>\
    struct mappable<C<Args...> &> : public default_mappable_container<C<Args...>> {};\
    template <class ... Args>\
    struct mappable<C<Args...> const> : public default_mappable_container<C<Args...>> {};\
    template <class ... Args>\
    struct mappable<C<Args...> const&> : public default_mappable_container<C<Args...>> {};\
    template <class ... Args>\
    struct mappable<C<Args...> &&> : public default_mappable_container<C<Args...>> {};

DEFAULT_MAPPABLE_CONTAINER_INSTANCE(std::deque);
DEFAULT_MAPPABLE_CONTAINER_INSTANCE(std::list);
DEFAULT_MAPPABLE_CONTAINER_INSTANCE(std::set);
DEFAULT_MAPPABLE_CONTAINER_INSTANCE(std::multiset);
DEFAULT_MAPPABLE_CONTAINER_INSTANCE(std::unordered_set);
DEFAULT_MAPPABLE_CONTAINER_INSTANCE(std::unordered_multiset);
DEFAULT_MAPPABLE_CONTAINER_INSTANCE(std::basic_string);
DEFAULT_MAPPABLE_CONTAINER_INSTANCE(std::vector);

#undef DEFAULT_MAPPABLE_CONTAINER_INSTANCE

    template <class F, class Ct, class ... Cts,
        typename = std::enable_if_t<funk::type_support::function_traits<F>::arity >= 2>>
    decltype(auto) map (F && f, Ct && ct, Cts && ... cts)
    {
        static_assert (funk::type_support::function_traits<F>::arity == 1 + sizeof...(cts),
                "arity of function does not match number of provided containers"); 
        using TB = decltype
            (funk::eval(f,
                        std::declval<typename funk::type_support::container_traits<Ct>::value_type>(),
                        std::declval<typename funk::type_support::container_traits<Cts>::value_type>()...));
        using OT = std::remove_cv_t<typename funk::type_support::container_traits<Ct>::template rebind<TB>>;
       
        OT out;
        auto const ends = std::make_tuple(ct.end(), (cts.end())...);
        for (auto&& its = std::make_tuple(ct.begin(), (cts.begin())...);
             !funk::utility::tuple_any_equal(its, ends);
             funk::utility::tuple_increment(its))
        {
            funk::type_support::container_traits<OT>::insert
                (out, funk::eval_tuple(f, funk::utility::tuple_dereference(its)));
        }
        return out;
    }

    template <class F, class T,
        typename = std::enable_if_t<funk::type_support::function_traits<F>::arity == 1>>
    inline constexpr decltype(auto) map (F && f, T && t)
    {
        return funk::mappable<T>::map (f, std::forward<T>(t));
    }
} // namespace funk

#endif // ifndef MAPPABLE_HPP

