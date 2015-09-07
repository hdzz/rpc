//
// Fixed points for nonrecursive 1-ary type constructors in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// LICENSE: Please see LICENSE.md
//

#ifndef FIX_TYPE_HPP
#define FIX_TYPE_HPP

#include <type_traits>

namespace fnk
{
    template <template <typename> class F>
    struct fix : F<fix<F>>
    {
        explicit constexpr fix (F<fix<F>> f) : F<fix<F>> (f) {}
        explicit constexpr fix (F<fix<F>> & f) : F<fix<F>> (f) {}
        explicit constexpr fix (F<fix<F>> const& f) : F<fix<F>> (f) {}
        explicit constexpr fix (F<fix<F>> && f) : F<fix<F>> (f) {}
    };

    template <template <typename> class F>
    static inline constexpr decltype(auto) fx (F<fix<F>> && f)
    {
        return fix<F> {std::forward<F<fix<F>>>(f)};
    }
    
    template <template <typename> class F>
    static inline constexpr decltype(auto) fx (F<fix<F>> const& f)
    {
        return fix<F> {f};
    }

    template <template <typename> class F>
    static inline constexpr F<fix<F>> unfx (fix<F> && f)
    {
        return std::forward<fix<F>>(f);
    }
    
    template <template <typename> class F>
    static inline constexpr F<fix<F>> unfx (fix<F> const& f)
    {
        return f;
    }
} // namespace fnk

#endif // ifndef FIX_TYPE_HPP

