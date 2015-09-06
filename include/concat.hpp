//
// Concatenate a container of monoid values
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef CONCAT_HPP
#define CONCAT_HPP

#include <type_traits>
#include <list>

#include "monoid.hpp"
#include "foldable.hpp"
#include "type_support/container_traits.hpp"

namespace funk
{
    template <class C,
        typename = std::enable_if_t<funk::rfoldable<C>::is_rfoldable_instance::value>,
        typename = std::enable_if_t
            <funk::monoid<typename funk::type_support::container_traits<C>::value_type>::is_monoid_instance::value>>
    inline constexpr decltype(auto) concat (C const& c)
    {
        using A = std::decay_t<typename funk::type_support::container_traits<C>::value_type>;
        return funk::rfoldable<C>::template foldr
            ([](auto&& l, auto&& r) { return funk::append<decltype(l),decltype(r)>(l, r); }, funk::monoid<A>::unity(), c); 
    }
} // namespace funk

#endif // ifndef CONCAT_HPP

