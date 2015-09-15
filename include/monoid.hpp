//
// Monoid Type Class
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef MONOID_HPP
#define MONOID_HPP

#include <cstdint>
#include <functional>
#include <type_traits>

#include <deque>
#include <list>
#include <string>
#include <vector>

#include "semigroup.hpp"
#include "type_support/container_traits.hpp"
#include "utility/type_utils.hpp"

namespace fnk
{
    template <typename T>
    struct monoid
    {
        // T unity() acts as multiplicative 1 (or additive 0 based on context).
        // T append() acts as multiplication (or addition based on context).
        struct is_monoid_instance : public std::false_type {};
    };

    template <typename T, typename A>
    struct default_monoid : public default_semigroup<T, A>
    {
        static inline constexpr decltype(auto) unity (void) { return T{}; }
        struct is_monoid_instance : public std::true_type {};
    };

    template <class C,
        typename = std::enable_if_t
            <fnk::monoid
                <typename fnk::type_support::container_traits<C>::value_type>::is_monoid_instance::value>>
    struct default_monoid_container : public default_semigroup_container<C>
    {
        static constexpr decltype(auto) unity (void) { return C{}; }
        struct is_monoid_instance : public std::true_type {};
    };

    template <typename T>
    decltype(auto) repeat (T && t, std::size_t const n)
    {
        auto u = fnk::monoid<T>::unity();
        for (std::size_t i = 0; i < n; ++i)
            u = fnk::monoid<T>::append (static_cast<T>(u), std::forward<T>(t));
        return u;
    }

#define DEFAULT_TYPES()\
    MONOID_INSTANCE(bool)\
    MONOID_INSTANCE(char)\
    MONOID_INSTANCE(wchar_t)\
    MONOID_INSTANCE(char16_t)\
    MONOID_INSTANCE(char32_t)\
    MONOID_INSTANCE(int8_t)\
    MONOID_INSTANCE(uint8_t)\
    MONOID_INSTANCE(int16_t)\
    MONOID_INSTANCE(uint16_t)\
    MONOID_INSTANCE(int32_t)\
    MONOID_INSTANCE(uint32_t)\
    MONOID_INSTANCE(int64_t)\
    MONOID_INSTANCE(uint64_t)\
    MONOID_INSTANCE(size_t)\
    MONOID_INSTANCE(float)\
    MONOID_INSTANCE(double)\
    MONOID_INSTANCE(long double)

#define MONOID_INSTANCE(T)\
    template<>\
    struct monoid<T> : public default_monoid<T, struct detail::default_append<T>> {};\
    template<>\
    struct monoid<T const> : public default_monoid<T, struct detail::default_append<T>> {};\
    template<>\
    struct monoid<T&> : public default_monoid<T, struct detail::default_append<T>> {};\
    template<>\
    struct monoid<T const&> : public default_monoid<T, struct detail::default_append<T>> {};\
    template<>\
    struct monoid<T&&> : public default_monoid<T, struct detail::default_append<T>> {};

    DEFAULT_TYPES();

#undef MONOID_INSTANCE
#undef DEFAULT_TYPES

#define DEFAULT_CONTAINERS()\
    MONOID_INSTANCE_CONTAINER(std::vector)\
    MONOID_INSTANCE_CONTAINER(std::deque)\
    MONOID_INSTANCE_CONTAINER(std::list)\
    MONOID_INSTANCE_CONTAINER(std::basic_string)

#define MONOID_INSTANCE_CONTAINER(C)\
    template <typename ... Args>\
    struct monoid<C<Args...>> : public default_monoid_container<C<Args...>> {};\
    template <typename ... Args>\
    struct monoid<C<Args...> const> : public default_monoid_container<C<Args...>> {};\
    template <typename ... Args>\
    struct monoid<C<Args...> &> : public default_monoid_container<C<Args...>> {};\
    template <typename ... Args>\
    struct monoid<C<Args...> const&> : public default_monoid_container<C<Args...>> {};\
    template <typename ... Args>\
    struct monoid<C<Args...> &&> : public default_monoid_container<C<Args...>> {};

DEFAULT_CONTAINERS();

#undef MONOID_INSTANCE_CONTAINER
#undef DEFAULT_CONTAINERS
} // namespace fnk

#endif // ifndef MONOID_HPP

