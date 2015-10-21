//
// Aggregation on data structures
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef FOLDABLE_HPP
#define FOLDABLE_HPP

#include <type_traits>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <tuple>
#include <vector>

#include "eval.hpp"
#include "monoid.hpp"
#include "type_support/container_traits.hpp"
#include "utility/type_utils.hpp"

namespace fnk
{
    //
    // foldlable (Foldable)
    //
    template <class T>
    struct foldable
    {
        // T value_type
        // A fold(foldable<A>) requires monoid<A>
        // B foldl(V(V,A),B,foldable<A>)
        struct is_foldable_instance : public std::false_type {}; 
    };

    template <class T>
    inline constexpr decltype(auto) fold (T && t)
    {
        return fnk::foldable<T>::fold (std::forward<T>(t));
    }

    template <class F, class B, class T>
    inline constexpr decltype(auto) foldl (F && f, B && b, T && t)
    {
        return fnk::foldable<T>::foldl (f, std::forward<B>(b), std::forward<T>(t));
    }

    template <class C>
    struct default_foldable
    {
        using value_type = typename fnk::type_support::container_traits<C>::value_type;
        
        template <class C_,
            typename = std::enable_if_t<std::is_convertible<C_,C>::value>,
            typename = std::enable_if_t
                <fnk::monoid<typename fnk::type_support::container_traits<C_>::value_type>::is_monoid_instance::value>>
        static inline constexpr decltype(auto) fold (C_ && c)
        {
            using U = fnk::utility::rebind_argument_t<C_, C>;
            using A = typename fnk::type_support::container_traits<U>::value_type;
            
            return fnk::default_foldable<U>::template foldl<decltype(&fnk::append<A>)>
                (&fnk::append<A>, fnk::monoid<A>::unity(), std::forward<U>(c));
        }

        template <class F, class B, class C_,
            typename = std::enable_if_t<std::is_convertible<C_,C>::value>,
            typename = std::enable_if_t
                <fnk::monoid<typename fnk::type_support::container_traits<C_>::value_type>::is_monoid_instance::value>>
        static constexpr decltype(auto) foldl (F && f, B && b, C_ && c)
        {
            using U = fnk::utility::rebind_argument_t<C_, C>;
            using W = fnk::utility::rebind_argument_t<std::remove_cv_t<B>, B>;

            auto b_ = std::forward<W>(b);
            for (auto const& e : std::forward<U>(c))
                b_ = f (e, b_);
            return b_;
        }

        struct is_foldable_instance : public std::true_type {};
    };

#define DEFAULT_FOLDABLE_INSTANCE(C)\
    template<typename ... Args>\
    struct foldable<C<Args...>> : public default_foldable<C<Args...>> {};\
    template<typename ... Args>\
    struct foldable<C<Args...> const> : public default_foldable<C<Args...>> {};\
    template<typename ... Args>\
    struct foldable<C<Args...> &> : public default_foldable<C<Args...>> {};\
    template<typename ... Args>\
    struct foldable<C<Args...> const&> : public default_foldable<C<Args...>> {};\
    template<typename ... Args>\
    struct foldable<C<Args...> &&> : public default_foldable<C<Args...>> {};

#define DEFAULT_CONTAINERS()\
    DEFAULT_FOLDABLE_INSTANCE(std::basic_string)\
    DEFAULT_FOLDABLE_INSTANCE(std::deque)\
    DEFAULT_FOLDABLE_INSTANCE(std::forward_list)\
    DEFAULT_FOLDABLE_INSTANCE(std::list)\
    DEFAULT_FOLDABLE_INSTANCE(std::vector)\
    DEFAULT_FOLDABLE_INSTANCE(std::set)\
    DEFAULT_FOLDABLE_INSTANCE(std::multiset)\
    DEFAULT_FOLDABLE_INSTANCE(std::unordered_set)\
    DEFAULT_FOLDABLE_INSTANCE(std::unordered_multiset)

    DEFAULT_CONTAINERS();

#undef DEFAULT_CONTAINERS
#undef DEFAULT_FOLDABLE_INSTANCE

    //
    // rfoldable
    //
    template <class T>
    struct rfoldable
    {
        // T value_type
        // B foldr (B(A,B), B, foldable<A>)
        struct is_rfoldable_instance : public std::false_type {};
    };

    template <class F, class B, class T>
    inline constexpr decltype(auto) foldr (F && f, B && b, T && t)
    {
        return fnk::rfoldable<T>::foldr (f, std::forward<B>(b), std::forward<T>(t));
    }

    template <class C>
    struct default_rfoldable : public default_foldable<C>
    {
        using value_type = typename fnk::type_support::container_traits<C>::value_type;

        template <class F, class B>
        static constexpr decltype(auto) foldr (F && f, B && b, C const& c)
        {
            auto it (c.crbegin());
            auto end (c.crend());
            auto b_ (std::forward<std::remove_cv_t<B>>(b));
            for (; it != end; ++it)
                b_ = f (*it, b_);
            return b_;
        }

        struct is_rfoldable_instance : public std::true_type {};
    };
 
#define DEFAULT_RFOLDABLE_INSTANCE(C)\
    template<typename ... Args>\
    struct rfoldable<C<Args...>> : public default_rfoldable<C<Args...>> {};\
    template<typename ... Args>\
    struct rfoldable<C<Args...> const> : public default_rfoldable<C<Args...>> {};\
    template<typename ... Args>\
    struct rfoldable<C<Args...> &> : public default_rfoldable<C<Args...>> {};\
    template<typename ... Args>\
    struct rfoldable<C<Args...> const&> : public default_rfoldable<C<Args...>> {};\
    template<typename ... Args>\
    struct rfoldable<C<Args...> &&> : public default_rfoldable<C<Args...>> {};

#define DEFAULT_CONTAINERS()\
    DEFAULT_RFOLDABLE_INSTANCE(std::basic_string)\
    DEFAULT_RFOLDABLE_INSTANCE(std::deque)\
    DEFAULT_RFOLDABLE_INSTANCE(std::list)\
    DEFAULT_RFOLDABLE_INSTANCE(std::vector)\
    DEFAULT_RFOLDABLE_INSTANCE(std::set)\
    DEFAULT_RFOLDABLE_INSTANCE(std::multiset)

    DEFAULT_CONTAINERS();

#undef DEFAULT_CONTAINERS
#undef DEFAULT_RFOLDABLE_INSTANCE
} // namespace fnk

#endif // FOLDABLE_HPP

