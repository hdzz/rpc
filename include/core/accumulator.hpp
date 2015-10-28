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

#include "../funktional/include/mappable.hpp"
#include "../funktional/include/filterable.hpp"
#include "../funktional/include/type_support/container_traits.hpp"

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
        using result_value_type = V;
        using backing_type = 
            std::deque <std::pair<result_type, range_type>>;
    private:
        using backing_value_t  = typename backing_type::value_type;
        using backing_ref_t    = typename backing_type::reference;
        using backing_cref_t   = typename backing_type::const_reference;
        using backing_size_t   = typename backing_type::size_type;
        using backing_iter_t   = typename backing_type::iterator;
        using backing_citer_t  = typename backing_type::const_iterator;
        using backing_riter_t  = typename backing_type::reverse_iterator;
        using backing_criter_t = typename backing_type::const_reverse_iterator;
    public:

        //
        // no default construction for accumulator
        //
        accumulator (void) = delete;

        //
        // okay to copy construct accumulators.
        //
        accumulator (accumulator &)               = default;
        accumulator (accumulator const&)          = default;

        //
        // okay to move accumulators.
        //
        accumulator (accumulator &&) = default;

        //
        // NOT okay to copy or move assign
        //
        accumulator & operator= (accumulator &&)     = delete;
        accumulator & operator= (accumulator const&) = delete;

        //
        // valid constructors
        //
        accumulator (result_type const& res, range_type const& rng)
        {
            acc_.emplace_back (res, rng);
        }

        accumulator (std::pair<result_type, range_type> const& p)
        {
            acc_.emplace_back (p);
        }

        // iterators:
        //
        inline backing_iter_t begin (void) noexcept
        {
            return acc_.begin ();
        }

        inline backing_iter_t end (void) noexcept
        {
            return acc_.end ();
        }

        inline backing_citer_t cbegin (void) const noexcept
        {
            return acc_.cbegin ();
        }
        
        inline backing_citer_t cend (void) const noexcept
        {
            return acc_.cend ();
        }
       
        // reverse iterators:
        //
        inline backing_riter_t rbegin (void) noexcept
        {
            return acc_.rbegin ();
        }
        
        inline backing_riter_t rend (void) noexcept
        {
            return acc_.rend ();
        }

        inline backing_criter_t crbegin (void) const noexcept
        {
            return acc_.crbegin ();
        }
        
        inline backing_criter_t crend (void) const noexcept
        {
            return acc_.crend ();
        }

        inline bool empty (void) const noexcept
        {
            return acc_.empty ();
        }

        inline backing_size_t size (void) const noexcept
        {
            return acc_.size ();
        }
 
        inline backing_type data (void) const noexcept
        {
            return acc_;
        }
        
        inline backing_cref_t data_view (void) const noexcept
        {
            return std::cref (acc_);
        }

        //
        // view the n'th most recent result.
        //
        inline backing_value_t view (std::size_t const n = 0) const noexcept
        {
            assert (not empty() &&
                    "cannot view into empty contianer (view call)");
            assert (n < size()  &&
                    "cannot view past end (view call)");

            auto it (cend ());
            return *std::prev (it, n + 1);
        }
 
        inline void insert (result_type const& res, range_type const& rng)
        {
            acc_.emplace_back (res, rng);
        }

        inline void insert (std::pair<result_type, range_type> const& p)
        {
            acc_.emplace_back (p);
        }

        inline void insert (accumulator const& other)
        {
            for (auto e : other.data ())
                acc_.emplace_back (e);
        }

        inline void ignore_previous (std::size_t const n = 1)
        {
            for (std::size_t i = 1; i <= n; ++i)
                acc_.pop_back ();
        }

        inline void replace (result_type const& res, range_type const& rng)
        {
            acc_.pop_back ();
            acc_.emplace_back (res, rng);
        }

        inline void replace (std::pair<result_type, range_type> const& p)
        {
            acc_.pop_back ();
            acc_.emplace_back (p);
        }

        inline void replace (result_type const& res)
        {
            auto rng (range ());
            replace (res, rng);
        }
 
        inline void replace (range_type const& rng)
        {
            auto res (result ());
            replace (res, rng);
        }

        //
        // obtain a view of the past n results in 
        // chronological order; returns a pair of 
        // iterators bounding this region.
        //
        inline std::pair<backing_citer_t, backing_citer_t> past
            (std::size_t const n) const noexcept
        {
            assert (not empty() &&
                    "cannot view into empty container (past call)");
            assert (n < size()  &&
                    "cannot view past end (past call)");
           
            auto to   (cend ());
            auto from (std::prev (to, n));
            return std::make_pair (from, to);
        }
 
        inline result_type result (std::size_t const n = 0) const noexcept
        {
            return view (n).first;
        }

        inline range_type range (std::size_t const n = 0) const noexcept
        {
            return view (n).second;
        }
 
        inline bool range_empty (std::size_t const n = 0) const noexcept
        {
            return view (n).second.empty ();
        }
 
        inline typename range_type::token_type range_head
            (std::size_t const n = 0) const noexcept
        {
            return range (n).head ();
        }
 
        inline range_type range_tail (std::size_t const n = 0) const noexcept
        {
            return range (n).tail ();
        }

        inline decltype(auto) values (void) const noexcept
        {
            using PT = std::pair<parse_result<V>, R>;

            return fnk::map
            (&result_type::to_value,
             fnk::filter
                (&result_type::is_value,
                 fnk::map (&PT::first, acc_)));
                    //([](std::pair<parse_result<V>, R> const& r) { return r.first; },
                    //acc_)));
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
        using result_value_type =
            typename A::result_value_type;
        using backing_type = typename A::backing_type;
    };
} // namespace core
} // namespace rpc

#endif // ifndef RANGE_HPP

