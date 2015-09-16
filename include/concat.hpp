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

namespace fnk
{
    template <class C>
    struct is_concatable_instance
        : std::conditional_t<fnk::rfoldable<C>::is_rfoldable_instance::value &&
          fnk::monoid<typename fnk::type_support::container_traits<C>::value_type>::is_monoid_instance::value,
          std::true_type, std::false_type> {};

    template <class C,
        typename = std::enable_if_t<fnk::rfoldable<C>::is_rfoldable_instance::value>,
        typename = std::enable_if_t
            <fnk::monoid<typename fnk::type_support::container_traits<C>::value_type>::is_monoid_instance::value>>
    inline constexpr decltype(auto) concat (C const& c)
    {
        using A = std::decay_t<typename fnk::type_support::container_traits<C>::value_type>;
        return fnk::rfoldable<C>::template foldr
            ([](auto&& l, auto&& r) { return fnk::append<decltype(l),decltype(r)>(l, r); }, fnk::monoid<A>::unity(), c); 
    }
} // namespace fnk

#endif // ifndef CONCAT_HPP

