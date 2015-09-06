//
// Accumulation on containers with monoid value types
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ACCUMULATE_HPP
#define ACCUMULATE_HPP

#include <type_traits>

#include "eval.hpp"
#include "monoid.hpp"
#include "type_support/container_traits.hpp"

namespace funk
{
    template <class C,
        typename = std::enable_if_t
            <funk::monoid<typename funk::type_support::container_traits<C>::value_type>::is_monoid_instance::value>>
    constexpr decltype(auto) accumulate (C && in)
    {
        using VT = typename funk::type_support::container_traits<C>::value_type;
     
        auto out (funk::monoid<VT>::unity());
        for (auto& e : std::forward<C>(in))
            out = funk::append (out, e);
        return out;
    }

    template <class F, class C,
        typename = std::enable_if_t
            <funk::monoid<typename funk::type_support::container_traits<C>::value_type>::is_monoid_instance::value>>
    constexpr decltype(auto) accumulate (C && in, F && f)
    {
        using VT = typename funk::type_support::container_traits<C>::value_type;

        auto out (funk::monoid<VT>::unity());
        for (auto const& e : std::forward<C>(in))
            out = funk::append (out, funk::eval(f,e));
        return out;
    }
} // namespace funk

#endif // ifndef ACCUMULATE_HPP

