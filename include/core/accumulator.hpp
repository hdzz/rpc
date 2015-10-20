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
        using type         = accumulator <It, V, R>;
        using range_type   = R;
        using result_type  = parse_result <V>;
        using backing_type = 
            std::deque <std::pair <result_type, range_type> const>;
 
        accumulator (void)
        {
            static_assert (sizeof(type) == 0,
                          "accumulator is not default constructable");
        }

        accumulator (result_type const& res, range_type const& rng)
        {
            acc_.emplace_front (res, rng);
        }

        inline decltype(auto) empty (void) const noexcept
        {
            return acc_.empty();
        }

        inline decltype(auto) size (void) const noexcept
        {
            return acc_.size();
        }
 
        inline decltype(auto) view (std::size_t const n = 0) const noexcept
        {
            assert (not empty() && "cannot view into empty contianer");
            assert (n < size()  && "cannot view past end");
            return acc_[n];
        }
 
        inline decltype(auto) data (void)
            const noexcept
        {
            return acc_;
        }
 
        inline decltype(auto) past (std::size_t const n)
            const noexcept
        {
            auto from (acc_.cbegin ());
            auto to (std::next (from, n));
            return backing_type (from, to);
        }
  
        inline decltype(auto) range (std::size_t const n = 0)
            const noexcept
        {
            return view (n).second;
        }
 
        inline decltype(auto) result (std::size_t const n = 0)
            const noexcept
        {
            return view (n).first;
        }

        inline decltype(auto) range_empty (std::size_t const n = 0)
            const noexcept
        {
            return view (n).second.empty();
        }
 
        inline decltype(auto) range_head (std::size_t const n = 0)
            const noexcept
        {
            return range (n).head();
        }
 
        inline decltype(auto) range_tail (std::size_t const n = 0)
            const noexcept
        {
            return range (n).tail();
        }
 
        inline decltype(auto) insert
            (result_type const& res, range_type const& rng)
        {
            acc_.emplace_front (res, rng);
        }

        inline decltype(auto) ignore_previous (void)
        {
            acc_.pop_front ();
        }

        inline decltype(auto) replace
            (result_type const& res, range_type const& rng)
        {
            acc_.pop_front ();
            acc_.emplace_front (res, rng);
        }
 
        inline decltype(auto) replace (result_type const& res)
        {
            auto rng (range ());
            return replace (res, rng);
        }
    private:
        backing_type acc_;
    };

    template <typename A>
    struct accumulator_traits
    {
        using type         = typename A::type;
        using range_type   = typename A::range_type;
        using result_type  = typename A::result_type;
        using backing_type = typename A::backing_type;
    };
} // namespace core
} // namespace rpc

#endif // ifndef RANGE_HPP

