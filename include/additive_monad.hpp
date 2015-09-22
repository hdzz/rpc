//
// Implementation of additive monads in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ADDITIVE_MONAD_HPP
#define ADDITIVE_MONAD_HPP

#include <type_traits>

#include "monad.hpp"
#include "monoid.hpp"

namespace fnk
{
    template <typename T>
    struct additive_monad
    {
        struct is_additive_monad_instance : public std::false_type {};
    };

    template <typename T,
        typename = std::enable_if_t<fnk::monad<T>::is_monad_instance::value>,
        typename = std::enable_if_t<fnk::monoid<T>::is_monoid_instance::value>> 
    struct default_additive_monad : public fnk::monad<T>, public fnk::monoid<T>
    {
        static inline constexpr decltype(auto) mzero (void) { return fnk::monoid<T>::unity (); }
       
        template <typename T_, typename = std::enable_if_t<std::is_convertible<T_, T>::value>> 
        static inline constexpr decltype(auto) mplus (T_ && l, T_ && r)
        {
            return fnk::monoid<T>::append (std::forward<T_>(l), std::forward<T_>(r));
        } 
        
        struct is_additive_monad_instance : public std::true_type {};
    };

    template <typename C,
        typename = std::enable_if_t<fnk::monad<C>::is_monad_instance::value>,
        typename = std::enable_if_t<fnk::monoid<C>::is_monoid_instance::value>>
    struct default_additive_monad_container : public default_additive_monad<C> {};
 
#define DEFAULT_ADDITIVE_MONAD_CONTAINER_INSTANCE(C)\
        template <class ... Args>\
        struct additive_monad<C<Args...>> : public default_additive_monad_container<C<Args...>> {};\
        template <class ... Args>\
        struct additive_monad<C<Args...> &> : public default_additive_monad_container<C<Args...>> {};\
        template <class ... Args>\
        struct additive_monad<C<Args...> const> : public default_additive_monad_container<C<Args...>> {};\
        template <class ... Args>\
        struct additive_monad<C<Args...> const&> : public default_additive_monad_container<C<Args...>> {};\
        template <class ... Args>\
        struct additive_monad<C<Args...> &&> : public default_additive_monad_container<C<Args...>> {};

    DEFAULT_ADDITIVE_MONAD_CONTAINER_INSTANCE(std::basic_string)
    DEFAULT_ADDITIVE_MONAD_CONTAINER_INSTANCE(std::deque)
    DEFAULT_ADDITIVE_MONAD_CONTAINER_INSTANCE(std::forward_list)
    DEFAULT_ADDITIVE_MONAD_CONTAINER_INSTANCE(std::list)
    DEFAULT_ADDITIVE_MONAD_CONTAINER_INSTANCE(std::vector)

#undef DEFAULT_ADDITIVE_MONAD_CONTAINER_INSTANCE

    //
    // Utility functions
    //
    template <typename M,
        typename = std::enable_if_t<additive_monad<M>::is_additive_monad_instance::value>>
    inline constexpr decltype(auto) mplus (M && l, M && r)
    {
        return additive_monad<M>::mplus (l, r);
    }
} // namespace fnk

#endif // ADDITIVE_MONAD_HPP

