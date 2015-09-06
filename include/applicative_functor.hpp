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

#include "functor.hpp"
#include "map.hpp"
#include "maybe.hpp"
#include "type_support/function_traits.hpp"

namespace fnk
{
    template <class T>
    struct applicative_functor;

    namespace detail
    {
        template <class T>
        struct not_applicative_functor
        {
            struct is_applicative_functor_instance : public std::false_type {};
        };
    } // namespace detail

    template <class T,
        typename = std::enable_if_t<fnk::functor<T>::is_functor_instance::value>>
    struct default_applicative_functor
    {
        struct is_applicative_functor_instance : public std::true_type {};
    };
} // namespace fnk

#endif // ifndef APPLICATIVE_FUNCTOR_HPP

