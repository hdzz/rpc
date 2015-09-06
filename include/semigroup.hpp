//
// Semigroup Type Class
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef SEMIGROUP_HPP
#define SEMIGROUP_HPP

#include <cstdint>
#include <functional>
#include <type_traits>

#include <deque>
#include <list>
#include <string>
#include <vector>

#include "type_support/container_traits.hpp"
#include "utility/type_utils.hpp"

namespace funk
{
    template <typename T>
    struct semigroup
    {
        // T append() acts as multiplication (or addition based on context).
        struct is_semigroup_instance : public std::false_type {};
    };

    template <typename T, typename A>
    struct default_semigroup 
    {
        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_, T>::value>>
        static inline constexpr decltype(auto) append (T_ && l, T_ && r)
        {
            using U = funk::utility::rebind_argument_t<T_,T>;
            return A::append (std::forward<U>(l), std::forward<U>(r));
        }
        
        struct is_semigroup_instance : public std::true_type {}; 
    };

    template <typename T>
    struct default_semigroup_container
    {
        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_, T>::value>>
        static inline constexpr decltype(auto) append (T_ && l, T_ && r)
        {
            using U = std::remove_cv_t<funk::utility::rebind_argument_t<T_,T>>;
            using W = funk::utility::rebind_argument_t<T_,T>;

            U out (std::forward<W>(l));
            for (auto&& e : std::forward<W>(r))
                funk::type_support::container_traits<U>::insert (out, e);
            return out;
        }
        struct is_semigroup_instance : public std::true_type {}; 
    };

    namespace detail
    {
        template <typename T>
        struct default_append
        {
            template <typename T_, typename = std::enable_if_t<std::is_convertible<T_, T>::value>>
            static inline constexpr decltype(auto) append (T_ && l, T_ && r)
            {
                using U = funk::utility::rebind_argument_t<T_,T>;
                return std::plus<T>() (std::forward<U>(l), std::forward<U>(r));
            }
        };
    } // namespace detail

    template <typename T1, typename T2,
        typename = std::enable_if_t
            <funk::semigroup<T1>::is_semigroup_instance::value && funk::semigroup<T2>::is_semigroup_instance::value>>
    inline constexpr decltype(auto) append (T1 && l, T2 && r)
    {
        using U = funk::utility::convert_between_t<T1, T2>;
        return funk::semigroup<U>::append (std::forward<U>(l), std::forward<U>(r));
    }

    template <typename T>
    inline constexpr decltype(auto) append (T && l, T && r)
    {
        return funk::semigroup<T>::append (std::forward<T>(l), std::forward<T>(r));
    }

#define DEFAULT_TYPES()\
    SEMIGROUP_INSTANCE(bool)\
    SEMIGROUP_INSTANCE(char)\
    SEMIGROUP_INSTANCE(wchar_t)\
    SEMIGROUP_INSTANCE(char16_t)\
    SEMIGROUP_INSTANCE(char32_t)\
    SEMIGROUP_INSTANCE(int8_t)\
    SEMIGROUP_INSTANCE(uint8_t)\
    SEMIGROUP_INSTANCE(int16_t)\
    SEMIGROUP_INSTANCE(uint16_t)\
    SEMIGROUP_INSTANCE(int32_t)\
    SEMIGROUP_INSTANCE(uint32_t)\
    SEMIGROUP_INSTANCE(int64_t)\
    SEMIGROUP_INSTANCE(uint64_t)\
    SEMIGROUP_INSTANCE(size_t)\
    SEMIGROUP_INSTANCE(float)\
    SEMIGROUP_INSTANCE(double)\
    SEMIGROUP_INSTANCE(long double)

#define SEMIGROUP_INSTANCE(T)\
    template<>\
    struct semigroup<T> : public default_semigroup<T, struct detail::default_append<T>> {};\
    template<>\
    struct semigroup<T const> : public default_semigroup<T, struct detail::default_append<T>> {};\
    template<>\
    struct semigroup<T&> : public default_semigroup<T, struct detail::default_append<T>> {};\
    template<>\
    struct semigroup<T const&> : public default_semigroup<T, struct detail::default_append<T>> {};\
    template<>\
    struct semigroup<T&&> : public default_semigroup<T, struct detail::default_append<T>> {};

    DEFAULT_TYPES();

#undef SEMIGROUP_INSTANCE
#undef DEFAULT_TYPES

#define DEFAULT_CONTAINERS()\
    SEMIGROUP_INSTANCE_CONTAINER(std::vector)\
    SEMIGROUP_INSTANCE_CONTAINER(std::deque)\
    SEMIGROUP_INSTANCE_CONTAINER(std::list)\
    SEMIGROUP_INSTANCE_CONTAINER(std::basic_string)

#define SEMIGROUP_INSTANCE_CONTAINER(C)\
    template <typename ... Args>\
    struct semigroup<C<Args...>> : public default_semigroup_container<C<Args...>> {};\
    template <typename ... Args>\
    struct semigroup<C<Args...> const> : public default_semigroup_container<C<Args...>> {};\
    template <typename ... Args>\
    struct semigroup<C<Args...> &> : public default_semigroup_container<C<Args...>> {};\
    template <typename ... Args>\
    struct semigroup<C<Args...> const&> : public default_semigroup_container<C<Args...>> {};\
    template <typename ... Args>\
    struct semigroup<C<Args...> &&> : public default_semigroup_container<C<Args...>> {};

DEFAULT_CONTAINERS();

#undef SEMIGROUP_INSTANCE_CONTAINER
#undef DEFAULT_CONTAINERS
} // namespace funk

#endif // ifndef SEMIGROUP_HPP

