//
// container flattening
//
// Author: Dalton Woodard
// Concact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef FLATTEN_HPP
#define FLATTEN_HPP

#include <type_traits>
#include <list>

#include "monoid.hpp"
#include "foldable.hpp"
#include "type_support/container_traits.hpp"
#include "utility/type_utils.hpp"

namespace funk
{
    template <class C,
        typename = std::enable_if_t<funk::foldable<C>::is_foldable_instance::value>>
    inline constexpr decltype(auto) flatten (C && c)
    {
    }
} // namesapce funk

auto r = funk::flatten (std::list<std::list<int>>{{1,2,3},{5,6,7}});

#endif // ifndef FLATTEN_HPP

