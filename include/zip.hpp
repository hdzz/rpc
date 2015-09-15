//
// zip and zip(with) implemnetations
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ZIP_HPP
#define ZIP_HPP

#include <type_traits>
#include <utility>

#include "defer.hpp"
#include "eval.hpp"
#include "type_support/container_traits.hpp"
#include "utility/tuple_utils.hpp"

namespace fnk
{
template <class C> 
struct is_zippable
    : public std::conditional_t
                <fnk::type_support::container_traits<C>::is_sequential::value, std::true_type, std::false_type>
    {};

namespace detail
{
    template <std::size_t I>
    struct unpack_tuple
    {
        template <class C>
        static constexpr decltype(auto) unpack (C && v)
        {
            using TupT = typename fnk::type_support::container_traits<C>::value_type;
            using ElmT = typename std::tuple_element<I, TupT>::type;
            using CT   = typename fnk::type_support::container_traits<C>::template rebind<ElmT>;
            CT out (v.size());
            for (auto const& e : std::forward<C>(v)) {
                out.push_back (std::get<I>(e));
            }
            return out;
        }
    };
    
    template <class C, std::size_t ... S>
    static inline constexpr decltype(auto) unzip_helper (C && v, fnk::utility::seq<S...>)
    {
        return std::make_tuple (unpack_tuple<S>::unpack(std::forward<C>(v))...);
    }
} // namespace detail

    template <class ... C>
    static constexpr decltype(auto) zip (C && ... cs)
    {
        using CT = std::tuple_element_t<0, std::tuple<C...>>;
        static_assert (fnk::is_zippable<CT>::value, "container type not zippable");
        
        using TupT = std::tuple<typename fnk::type_support::container_traits<C>::value_type...>;
        using OT = typename fnk::type_support::container_traits<CT>::template rebind<TupT>;
        
        OT out {};
        auto const ends = std::make_tuple (std::forward<C>(cs).end()...);
        for (auto iter = std::make_tuple (std::forward<C>(cs).begin()...);
            !fnk::utility::tuple_any_equal(iter, ends);
             fnk::utility::tuple_increment(iter))
        {
            fnk::type_support::container_traits<OT>::insert (out, fnk::utility::tuple_dereference(iter));
        }
        return out; 
    }

    template <class F, class ... C>
    static constexpr decltype(auto) zip_with (F && f, C && ... cs)
    {
        using CT = fnk::utility::index_type_t<0, C...>;
        static_assert (fnk::is_zippable<CT>::value, "container type not zippable");
        
        using OT = typename fnk::type_support::container_traits<CT>::template
            rebind<typename fnk::type_support::function_traits<F>::return_type>;
        
        OT out {};
        auto const ends = std::make_tuple (std::forward<C>(cs).end()...);
        for (auto iter = std::make_tuple (std::forward<C>(cs).begin()...);
            !fnk::utility::tuple_any_equal(iter, ends);
             fnk::utility::tuple_increment(iter))
        {
            fnk::type_support::container_traits<OT>::insert
                (out, fnk::eval_tuple
                    (f, fnk::utility::tuple_dereference(iter)));
        }
        return out; 
    }

    template <class F, class ... C>
    static constexpr decltype(auto) zip_with_defer (F && f, C && ... cs)
    {
        using CT = std::tuple_element_t<0, std::tuple<C...>>;
        static_assert (fnk::is_zippable<CT>::value, "container type not zippable");
        
        using AT = typename fnk::type_support::container_traits<CT>::template
            rebind<std::tuple<typename fnk::type_support::container_traits<C>::value_type...>>;
       
        using OT = typename fnk::type_support::container_traits<AT>::template
            rebind<std::result_of_t
                <decltype(fnk::defer_tuple<F, typename fnk::type_support::container_traits<AT>::value_type>)
                    (F, typename fnk::type_support::container_traits<AT>::value_type)>>; 
        
        OT out {};
        auto const ends = std::make_tuple (std::forward<C>(cs).end()...);
        for (auto iter = std::make_tuple (std::forward<C>(cs).begin()...);
            !fnk::utility::tuple_any_equal(iter, ends);
             fnk::utility::tuple_increment(iter))
        {
            fnk::type_support::container_traits<OT>::insert
                (out, fnk::defer_tuple
                   (f, fnk::utility::tuple_dereference(iter)));
        }
        return out; 
    }
    
    template <class C,
        typename = std::enable_if_t
            <fnk::utility::is_specialization
                <typename fnk::type_support::container_traits<C>::value_type, std::tuple>::value>>
    static constexpr decltype(auto) unzip (C && v)
    {
        using VT = typename fnk::type_support::container_traits<C>::value_type;
        return detail::unzip_helper (std::forward<C>(v), typename utility::seq_gen<std::tuple_size<VT>::value>::type());
    }
} // namesapce fnk

#endif // ifndef ZIP_HPP

