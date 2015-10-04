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
    template <typename IterT,
        typename = std::enable_if_t
            <std::is_base_of<std::bidirectional_iterator_tag,
                             typename std::iterator_traits<IterT>::iterator_category>::value>>
    struct range
    {
    public:
        using iter_type   = IterT;
        using iter_traits = std::iterator_traits<iter_type>;
        using token_type  = typename iter_traits::value_type;
        using diff_type   = typename iter_traits::difference_type;
        
        struct is_range_type : public std::true_type {};
        
        range (void) { static_assert (sizeof(IterT) == 0, "ranges are not defaul constructable"); }
    
        template <typename C,
            typename = std::enable_if_t<fnk::type_support::container_traits<C>::is_container::value>,
            typename = std::enable_if_t<std::is_same<token_type, typename fnk::type_support::container_traits<C>::value_type>::value>>
        constexpr range (C const& c) noexcept : valid_(not c.empty()), begin_(c.begin()), end_(c.end()) {}

        template <typename It,
            typename = std::enable_if_t<std::is_base_of<iter_type, It>::value>>
        constexpr range (It const& b, It const& e) noexcept : valid_(0 < std::distance(b, e)), begin_(b), end_(e) {}

        inline constexpr decltype(auto) begin (void) const noexcept { return begin_; }

        inline constexpr decltype(auto) cbegin (void) const noexcept { return begin_; }

        inline constexpr decltype(auto) end (void) const noexcept { return end_; }

        inline constexpr decltype(auto) cend (void) const noexcept { return end_; }
        
        inline constexpr decltype(auto) head (void) const noexcept(noexcept(*std::declval<const iter_type>())) { return *begin_; }
        
        inline constexpr decltype(auto) tail (void) const noexcept { return range (std::next(begin_), end_); }
    
        inline constexpr decltype(auto) tail (diff_type const n) const noexcept { return range (std::next(begin_, n), end_); }

        inline constexpr decltype(auto) length (void) const noexcept { return std::distance (begin_, end_); }
        
        inline constexpr decltype(auto) is_empty (void) const noexcept { return not (valid_ && begin_ != end_); }

        inline constexpr decltype(auto) is_valid (void) const noexcept { return valid_; }
   
    private:
        bool valid_;
        iter_type const begin_;
        iter_type const end_;
    };
} // namespace core
} // namespace rpc

#endif // ifndef RANGE_HPP

