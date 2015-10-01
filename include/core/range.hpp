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
    // A view into the sequence of tokens to be parsed.
    //
    template <typename IterT,
        typename = std::enable_if_t
            <std::is_base_of<std::bidirectional_iterator_tag,
                             typename std::iterator_traits<IterT>::iterator_category>::value>>
    struct range
    {
    public:
        using iter_type = IterT;
        using traits = std::iterator_traits<iter_type>;
        using token_type = typename traits::value_type;
        using diff_type = typename traits::difference_type;
        
        range (void) { static_assert (sizeof(IterT) == 0, "range is not defaul constructable"); }
    
        template <typename C,
            typename = std::enable_if_t<fnk::type_support::container_traits<C>::is_container::value>,
            typename = std::enable_if_t<std::is_same<token_type, typename fnk::type_support::container_traits<C>::value_type>::value>>
        constexpr range (C const& c) noexcept : valid(not c.empty()), start(c.begin()), end(c.end()) {}

        template <typename It,
            typename = std::enable_if_t<std::is_base_of<iter_type, It>::value>>
        constexpr range (It const& s, It const& e) noexcept : valid(0 < std::distance(s, e)), start(s), end(e) {}

        inline constexpr decltype(auto) getstart (void) const noexcept { return start; }

        inline constexpr decltype(auto) getend (void) const noexcept { return end; }

        inline constexpr decltype(auto) head (void) const noexcept(noexcept(*std::declval<const iter_type>())) { return *start; }
        
        inline constexpr decltype(auto) tail (void) const noexcept { return range (std::next(start), end); }
    
        inline constexpr decltype(auto) tail (diff_type const n) const noexcept { return range (std::next(start, n), end); }

        inline constexpr decltype(auto) length (void) const noexcept { return std::distance (start, end); }
        
        inline constexpr decltype(auto) is_empty (void) const noexcept { return not (valid && start != end); }

        inline constexpr decltype(auto) is_valid (void) const noexcept { return valid; }
    
    private:
        bool valid;
        iter_type const start;
        iter_type const end; 
    };
} // namespace core
} // namespace rpc

#endif // ifndef RANGE_HPP

