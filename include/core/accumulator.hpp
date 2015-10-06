//
// Accumulator type, a sequence of parser results
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ACCUMULATOR_HPP
#define ACCUMULATOR_HPP

#include <cassert>
#include <deque>
#include <type_traits>
#include <utility>

#include "range.hpp"
#include "result_type.hpp"

#include "funktional/include/type_support/container_traits.hpp"

namespace rpc
{
namespace core
{
    template <typename It, typename V, typename R = range<It>>
    struct accumulator
    {
    public:
        using type        = accumulator <It, V, R>;
        using range_type  = R;
        using result_type = parse_result <V>;
        
        accumulator (void) { static_assert (sizeof(type) == 0, "accumulator is not default constructable"); }

        inline decltype(auto) empty (void) const noexcept
        {
            return acc_.empty();
        }

        inline decltype(auto) size (void) const noexcept
        {
            return acc_.size();
        }
        
        inline decltype(auto) view (void) const noexcept
        {
            assert (not empty() && "cannot view into empty accumulator");
            return acc_.back();
        }
     
        inline decltype(auto) view (std::size_t const n) const noexcept
        {
            assert (not n < size() && "cannot view past end");
            return acc_.get(n);
        }
  
        inline decltype(auto) range (void) const noexcept
        {
            return view ().second;
        }

        inline decltype(auto) range (std::size_t const n) const noexcept
        {
            return view (n).second;
        }
 
        inline decltype(auto) result (void) const noexcept
        {
            return view().first;
        }
        
        inline decltype(auto) result (std::size_t const n) const noexcept
        {
            return view (n).first;
        }
        
        inline decltype(auto) range_head (void) const noexcept
        {
            return range().head();
        }
        
        inline decltype(auto) range_tail (void) const noexcept
        {
            return range().tail();
        }
 
        inline decltype(auto) insert (result_type const& res, range_type const& rng)
        {
            acc_.emplace_back (res, rng);
            return this;
        }
    private:
        std::deque <std::pair <result_type, range_type> const> acc_;
    };

    template <typename A>
    struct accumulator_traits
    {
        using type        = typename A::type;
        using range_type  = typename A::range_type;
        using result_type = typename A::result_type;
    };
} // namespace core
} // namespace rpc

#endif // ifndef RANGE_HPP

