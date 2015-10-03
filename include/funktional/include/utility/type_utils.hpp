//
// Some utility code for type manipulations
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef TYPE_UTILS_HPP
#define TYPE_UTILS_HPP

#include <tuple>
#include <type_traits>
#include <utility>

namespace fnk
{
namespace utility
{
    //
    // Utility type tag (essentially a boxed type for manual overloading)
    //
    template <typename ... Ts> struct type_tag {};

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

    template <typename T, template <typename ...> class Tmp>
    struct is_specialization : std::false_type {};
    
    template <template <typename ...> class Tmp, typename ... Ts>
    struct is_specialization<Tmp<Ts...>, Tmp> : std::true_type {};

    template <template <typename ...> class Tmp, typename ... Ts>
    struct is_specialization<Tmp<Ts...> const, Tmp> : std::true_type {};
    
    template <template <typename ...> class Tmp, typename ... Ts>
    struct is_specialization<Tmp<Ts...> &, Tmp> : std::true_type {};
    
    template <template <typename ...> class Tmp, typename ... Ts>
    struct is_specialization<Tmp<Ts...> const&, Tmp> : std::true_type {};
    
    template <template <typename ...> class Tmp, typename ... Ts>
    struct is_specialization<Tmp<Ts...> &&, Tmp> : std::true_type {};
    
    namespace detail
    {
        template <typename F, typename ... Args>
        struct is_good_call
        {
        private:
            template<typename F_, typename... Args_>
            static constexpr auto test (int) ->
                decltype(std::declval<F_>()(std::declval<Args_>()...), std::true_type());

            template<typename F_, typename... Args_>
            static constexpr std::false_type test (...);
        public:
            static constexpr bool value = decltype(test<F, Args...>(0)) {};
        };
    }
    
    template <typename F, typename ... Args>
    struct is_well_formed : public std::conditional<detail::is_good_call<F, Args...>::value, std::true_type, std::false_type> {};


    template <class T>
    inline constexpr T&& forward_constexpr (T & t) noexcept { return static_cast<T&&> (t); }

    template <class T>
    inline constexpr T&& forward_constexpr (T const& t) noexcept { return static_cast<T&&> (static_cast<T>(t)); }
    
    template <class T>
    inline constexpr T&& forward_constexpr (T && t) noexcept { return static_cast<T&&> (t); }

    template <class T>
    inline constexpr std::remove_reference_t<T>&& move_constexpr (T && t) noexcept
    { 
        return static_cast<std::remove_reference_t<T>&&> (t);
    }

    //
    // Index from type(s)
    //
    template <typename U, typename ... Ts>
    struct type_to_index;

    template <typename U, typename ... Ts>
    struct type_to_index <U, U, Ts...> : public std::integral_constant<std::size_t, 0> {};

    template <typename U, typename T, typename ... Ts>
    struct type_to_index <U, T, Ts...> : public std::integral_constant<std::size_t, 1 + type_to_index<U, Ts...>::value> {};
   
    template <typename U>
    struct type_to_index<U> { static_assert (sizeof(U) == 0, "type not found in list"); };

    //
    // Type from index
    //
    template <std::size_t I, typename ... Ts>
    using index_type = std::tuple_element<I, std::tuple<Ts...>>;

    template <std::size_t I, typename ... Ts>
    using index_type_t = std::tuple_element_t<I, std::tuple<Ts...>>;

    //
    // Building parameter type packs
    //
    template <std::size_t Nt, typename Tup>
    struct types
    {
        template <std::size_t N>
        using type = std::remove_reference_t<decltype(std::get<N>(std::declval<Tup>()))>;
    };
} // namespace utility
} // naemspace fnk

#endif // ifndef TYPE_UTILS_HPP

