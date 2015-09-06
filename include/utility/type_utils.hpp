//
// Some utility code for type manipulations
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef TYPE_UTILS_HPP
#define TYPE_UTILS_HPP

#include <type_traits>
#include <utility>

namespace fnk
{
namespace utility
{
    //
    // Utility type tag (essentially a boxed type for manual overloading)
    //
    template <typename T> struct type_tag {};

    //
    // Choose correct rebinding for perfect forwarding, preserving constness
    //
    template <typename T1, typename T2>
    using rebind_argument_t =
        std::remove_reference_t
            <std::conditional_t
                <std::is_const<std::remove_reference_t<T1>>::value, T1, T2>>; 

    //
    // Take first possible convertion between two types, if it exists
    //
    template <typename T1, typename T2,
        typename = std::enable_if_t<std::is_convertible<T1,T2>::value || std::is_convertible<T2,T1>::value>>
    using convert_between_t = std::conditional_t<std::is_convertible<T1,T2>::value, T2, T1>;

    //
    // Strip references and constness, but not full decay
    //
    template <typename T>
    using remove_cv_r_t = std::remove_cv_t<std::remove_reference_t<T>>;

    //
    // Positive integer sequences
    //
    template <std::size_t ...>
    struct seq {};

    template <std::size_t N, std::size_t ... S>
    struct seq_gen : seq_gen <N-1, N-1, S...> {};

    template <std::size_t ... S>
    struct seq_gen <0, S...>
    {
        using type = seq<S...>;
    };

    //
    // Type equality
    //
    template <class T, class ... Us>
    struct is_same
        : std::conditional_t<0 == sizeof...(Us),
                             std::true_type, 
                             std::conditional_t<is_same<Us...>::value, std::true_type, std::false_type>> {};

    template <class T>
    struct is_same<T,T> : std::true_type {};

    namespace detail
    {
        template <typename T>
        using constant_void = void;

        template <typename E, typename Enable = void>
        struct is_well_formed : public std::false_type {};

        template <typename F_, typename ... Args_>
        struct is_well_formed <F_(Args_...), constant_void<std::result_of_t<F_(Args_...)>>>
            : public std::true_type {};
    }

    template <typename E>
    struct is_well_formed : public detail::is_well_formed<E> {};
} // namespace utility
} // naemspace fnk

#endif // ifndef TYPE_UTILS_HPP

