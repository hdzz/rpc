//
// Some utility code for boolean manipulations
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef BOOL_UTILS_HPP
#define BOOL_UTILS_HPP

#include <type_traits>

namespace fnk
{
namespace utility
{
    //
    // any_true (logical or)
    //
    static inline constexpr bool any_true (bool const& b) { return b; }

    template <typename ... BOOL>
    static constexpr bool any_true (bool const& b1, BOOL && ... bs)
    {
        return b1 || any_true (std::forward<BOOL>(bs)...);
    }

    template <typename B, typename ... Bs,
        typename std::enable_if<std::is_convertible<B, bool>::value>::type, 
        typename std::enable_if<std::is_convertible<Bs, bool>::value>::type...>
    static constexpr bool any_true (B && b1, Bs&& ... bs) { return std::forward<B>(b1) || any_true(std::forward<Bs>(bs)...); }
   
    //
    // all_true (logical and)
    //
    static inline constexpr bool all_true (bool const& b) { return b; }
    
    template <typename ... BOOL>
    static constexpr bool all_true (bool const& b1, BOOL&& ... bs)
    { 
        return b1 && all_true (std::forward<BOOL>(bs)...);
    }

    template <typename B, typename ... Bs,
        typename std::enable_if<std::is_convertible<B, bool>::value>::type, 
        typename std::enable_if<std::is_convertible<Bs, bool>::value>::type...>
    static constexpr bool all_true (B && b1, Bs&& ... bs) { return std::forward<B>(b1) && all_true(std::forward<Bs>(bs)...); }
} // namespace utility
} // naemspace fnk

#endif // ifndef BOOL_UTILS_HPP

