//
// Range type, a view into a sequence of tokens
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef RANGE_HPP
#define RANGE_HPP

#include <iterator>
#include <type_traits>

#include "funktional/include/type_support/container_traits.hpp"

namespace rpc
{
namespace core
{
    //
    // A view into the sequence of tokens to be parsed. If
    // additional information is needed (such as file line numbers)
    // to handle parse errors, simply inherit from this struct
    // and define the appropriate methods.
    //
    template <typename It,
        typename = std::enable_if_t
            <std::is_base_of
                <std::bidirectional_iterator_tag,
                typename std::iterator_traits<It>::iterator_category>
            ::value>>
    struct range
    {
    public:
        using type        = range<It>;
        using iter_type   = It;
        using iter_traits = std::iterator_traits<iter_type>;
        using token_type  = typename iter_traits::value_type;
        using diff_type   = typename iter_traits::difference_type;

        struct is_range_type : public std::true_type {};

        //
        // no default construction for a range.
        //
        range (void) = delete;

        //
        // okay to copy construct ranges.
        //
        range (range &)               = default;
        range (range const&)          = default;

        //
        // okay to move ranges.
        //
        range (range &&) = default;

        //
        // NOT okay to copy or move assign
        //
        range & operator= (range &&)     = delete;
        range & operator= (range const&) = delete;

        //
        // valid constructors
        //
        template <typename C>
        range (C const& c) noexcept
            : valid_  (not c.empty()),
              begin_  (c.cbegin()),
              end_    (c.cend()),
              length_ (std::distance(begin_, end_))
        {}

        template <typename I>
        range (I const& b, I const& e) noexcept
            : valid_  (0 < std::distance(b, e)),
              begin_  (b),
              end_    (e),
              length_ (std::distance(begin_, end_))
        {}

        inline constexpr iter_type begin (void) const noexcept
            { return begin_; }

        inline constexpr iter_type cbegin (void) const noexcept
            { return begin_; }

        inline constexpr iter_type end (void) const noexcept
            { return end_; }

        inline constexpr iter_type cend (void) const noexcept
            { return end_; }

        inline constexpr auto head (void)
            const noexcept(noexcept(*std::declval<const iter_type>()))
            -> decltype(*std::declval<const iter_type>())
        { return *begin_; }
 
        inline type tail (diff_type const n = 1) const noexcept
        {
            return length_ >= n ? range (std::next(begin_, n), end_)
                                : range (end_, end_);
        }

        inline diff_type length (void) const noexcept
            { return std::distance (begin_, end_); }

        inline diff_type distance (range<It> const& other)
            { return std::distance (begin(), other.begin()); }

        inline constexpr bool empty (void) const noexcept
            { return not (valid_ && length_ > 0); }

        inline constexpr bool valid (void) const noexcept
            { return valid_; }

        inline std::basic_string<token_type> grab (void) const
        {
            std::basic_stringstream<token_type> st;
            for (auto it = begin_; it != end_; ++it)
                st << *it;
            return st.str ();
        }

        inline std::basic_string<token_type> grab
            (std::size_t n) const
        {
            std::basic_stringstream<token_type> st;
            for (auto it = begin_; it != end_ && n-- != 0; ++it)
                st << *it;
            return st.str ();
        }

    private:
        bool const valid_;
        iter_type const begin_;
        iter_type const end_;
        diff_type const length_;
    };

    template <typename R>
    struct range_traits
    {
        using iter_type   = typename R::iter_type;
        using iter_traits = typename R::iter_traits;
        using token_type  = typename iter_traits::value_type;
        using diff_type   = typename iter_traits::difference_type;
    };
} // namespace core
} // namespace rpc

#endif // ifndef RANGE_HPP
