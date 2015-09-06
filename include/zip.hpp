//
// zip and zip(with) implemnetations
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ZIP_HPP
#define ZIP_HPP

#include <cassert>
#include <list>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "defer.hpp"
#include "eval.hpp"
#include "foldable.hpp"
#include "mappable.hpp"
#include "type_support/container_traits.hpp"
#include "type_support/function_traits.hpp"
#include "utility/bool_utils.hpp"
#include "utility/tuple_utils.hpp"

namespace funk
{
template <class C> 
struct is_zippable
    : public std::conditional_t
                <funk::type_support::container_traits<C>::is_sequential::value, std::true_type, std::false_type>
    {};

namespace detail
{
    template <std::size_t I>
    struct unpack_tuple
    {
/*        template <template <class...> class C, class ... Ts>
        static constexpr decltype(auto) unpack (C<std::tuple<Ts...>> & v)
        { 
            using TupT = typename std::tuple<Ts...>;
            using ElmT = typename funk::utils::tuple_helper<I>::template elem_t<Ts...>;
            C<ElmT> out (v.size());
            for (auto const& e : v) {
                out.push_back (std::get<I>(e));
            }
            return out;
        }
*/        
        //template <template <class...> class C, class ... Ts>
        template <class C>
        static constexpr decltype(auto) unpack (C && v)
        {
            using TupT = typename funk::type_support::container_traits<C>::value_type;
            using ElmT = typename std::tuple_element<I, TupT>::type;
            using CT   = typename funk::type_support::container_traits<C>::template rebind<ElmT>;
            CT out (v.size());
            for (auto const& e : std::forward<C>(v)) {
                out.push_back (std::get<I>(e));
            }
            return out;
        }
/*        
        template <template <class...> class C, class ... Ts>
        static constexpr decltype(auto) unpack (C<std::tuple<Ts...>> const& v)
        { 
            using TupT = typename std::tuple<Ts const...>;
            using ElmT = typename funk::utils::tuple_helper<I>::template elem_t<Ts const...>;
            C<ElmT> out (v.size());
            for (auto const& e : v) {
                out.push_back (std::get<I>(e));
            }
            return out;
        }

        template <template <class...> class C, class ... Ts>
        static constexpr decltype(auto) unpack (C<std::tuple<Ts...>> const&& v)
        { 
            using TupT = typename std::tuple<Ts const...>;
            using ElmT = typename funk::utils::tuple_helper<I>::template elem_t<Ts const...>;
            C<ElmT> out (std::forward<C<std::tuple<Ts const...>>>(v).size());
            for (auto const& e : std::forward<C<std::tuple<Ts...>> const>(v)) {
                out.push_back (std::get<I>(e));
            }
            return out;
        }
*/
    };
/*
    template <template <class...> class C, class ... Ts, std::size_t ... S>
    static constexpr decltype(auto) unzip_helper (C<std::tuple<Ts...>> & v, funk::utils::seq<S...>)
    {
        return std::make_tuple (unpack_tuple<S>::unpack(v)...);
    }
*/
    //template <template <class...> class C, class ... Ts, std::size_t ... S>
    //static constexpr decltype(auto) unzip_helper (C<std::tuple<Ts...>> && v, funk::utils::seq<S...>)
    template <class ... C, std::size_t ... S>
    static inline constexpr decltype(auto) unzip_helper (C && ... v, funk::utility::seq<S...>)
    {
        static_assert (funk::utility::is_same<std::decay_t<C>...>::value, "container types must be the same");
        return std::make_tuple (unpack_tuple<S>::unpack(std::forward<C>(v))...);
    }
/*
    template <template <class...> class C, class ... Ts, std::size_t ... S>
    static constexpr decltype(auto) unzip_helper (C<std::tuple<Ts...>> const& v, funk::utils::seq<S...>)
    {
        return std::make_tuple (unpack_tuple<S>::unpack(v)...);
    }

    template <template <class...> class C, class ... Ts, std::size_t ... S>
    static constexpr decltype(auto) unzip_helper (C<std::tuple<Ts...>> const&& v, funk::utils::seq<S...>)
    {
        return std::make_tuple (unpack_tuple<S>::unpack(std::forward<C<std::tuple<Ts...>> const>(v))...);
    }
*/
} // namespace detail
/*
template <template <class...> class C, class ... Ts>
static constexpr decltype(auto) zip (C<Ts>& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using CT = C<std::tuple<Ts...>>;
    CT out {};
    auto const ends = std::make_tuple (cs.end()...);
    for (auto iter = std::make_tuple (cs.begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert (out, funk::utils::tuple_dereference(iter));
    }
    return out; 
}
*/
//template <template <class...> class C, class ... Ts>
//static constexpr decltype(auto) zip (C<Ts>&& ... cs)
template <class ... C>
static constexpr decltype(auto) zip (C && ... cs)
{
    static_assert
        (funk::utility::all_true((funk::is_zippable<C>::value)...), "container type not zippable");
    using CT = C<std::tuple<Ts...>>;
    CT out {};
    auto const ends = std::make_tuple (std::forward<C<Ts>>(cs).end()...);
    for (auto iter = std::make_tuple (std::forward<C<Ts>>(cs).begin()...);
        !funk::utility::tuple_any_equal(iter, ends);
         funk::utility::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert (out, funk::utility::tuple_dereference(iter));
    }
    return out; 
}
/*
template <template <class...> class C, class ... Ts>
static constexpr decltype(auto) zip (C<Ts> const& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using CT = C<std::tuple<Ts...>> const;
    CT out {};
    auto const ends = std::make_tuple (cs.end()...);
    for (auto iter = std::make_tuple (cs.begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert (out, funk::utils::tuple_dereference(iter));
    }
    return out; 
}

template <template <class...> class C, class ... Ts>
static constexpr decltype(auto) zip (C<Ts> const&& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using CT = C<std::tuple<Ts...>> const;
    CT out {};
    auto const ends = std::make_tuple (std::forward<C<Ts>>(cs).end()...);
    for (auto iter = std::make_tuple (std::forward<C<Ts>>(cs).begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert (out, funk::utils::tuple_dereference(iter));
    }
    return out; 
}
*/
/*
template <template <class...> class C, class F, class ... Ts>
static constexpr decltype(auto) zip_with (F && f, C<Ts>& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using CT = C<typename funk::type_support::function_traits<F>::return_type>;
    CT out {};
    auto const ends = std::make_tuple (cs.end()...);
    for (auto iter = std::make_tuple (cs.begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert
            (out, funk::eval_tuple
                (f, funk::utils::tuple_dereference(iter)));
    }
    return out; 
}
*/
template <class ... C, class F>
static constexpr decltype(auto) zip_with (F && f, C && ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<std::decay_t<C>>::value)...), "container type not zippable");
    using CT = typename funk::type_support::container_traits<C>::template
                    rebind<typename funk::type_support::function_traits<F>::return_type>::type; 
    CT out {};
    auto const ends = std::make_tuple (std::forward<C<Ts>>(cs).end()...);
    for (auto iter = std::make_tuple (std::forward<C<Ts>>(cs).begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert
            (out, funk::eval_tuple
                (f, funk::utils::tuple_dereference(iter)));
    }
    return out; 
}
/*
template <template <class...> class C, class F, class ... Ts>
static constexpr decltype(auto) zip_with (F && f, C<Ts> const& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using CT = C<typename funk::type_support::function_traits<F>::return_type>;
    CT out {};
    auto const ends = std::make_tuple (cs.end()...);
    for (auto iter = std::make_tuple (cs.begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert
            (out, funk::eval_tuple
                (f, funk::utils::tuple_dereference(iter)));
    }
    return out; 
}

template <template <class...> class C, class F, class ... Ts>
static constexpr decltype(auto) zip_with (F && f, C<Ts> const&& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using CT = C<typename funk::type_support::function_traits<F>::return_type>;
    CT out {};
    auto const ends = std::make_tuple (std::forward<C<Ts>>(cs).end()...);
    for (auto iter = std::make_tuple (std::forward<C<Ts>>(cs).begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert
            (out, funk::eval_tuple
                (f, funk::utils::tuple_dereference(iter)));
    }
    return out; 
}
*/
/*
template <template <class...> class C, class F, class ... Ts>
static constexpr decltype(auto) zip_with_defer (F && f, C<Ts>& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using RT = typename funk::type_support::function_traits<F>::return_type;
    using CT = C<std::function<RT(void)>>;
    CT out {};
    auto const ends = std::make_tuple (cs.end()...);
    for (auto iter = std::make_tuple (cs.begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert
            (out, funk::apply_tuple
                (f, funk::utils::tuple_dereference(iter)));
    }
    return out; 
}
*/
template <template <class...> class C, class F, class ... Ts>
static constexpr decltype(auto) zip_with_defer (F && f, C<Ts>&& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using RT = typename funk::type_support::function_traits<F>::return_type;
    using CT = C<std::function<RT(void)>>;
    CT out {};
    auto const ends = std::make_tuple (std::forward<C<Ts>>(cs).end()...);
    for (auto iter = std::make_tuple (std::forward<C<Ts>>(cs).begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert
            (out, funk::apply_tuple
               (f, funk::utils::tuple_dereference(iter)));
    }
    return out; 
}
/*
template <template <class...> class C, class F, class ... Ts>
static constexpr decltype(auto) zip_with_apply (F && f, C<Ts> const& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using RT = typename funk::type_support::function_traits<F>::return_type;
    using CT = C<std::function<RT(void)>>;
    CT out {};
    auto const ends = std::make_tuple (cs.end()...);
    for (auto iter = std::make_tuple (cs.begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert
            (out, funk::apply_tuple
                (f, funk::utils::tuple_dereference(iter)));
    }
    return out; 
}

template <template <class...> class C, class F, class ... Ts>
static constexpr decltype(auto) zip_with_apply (F && f, C<Ts> const&& ... cs)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not zippable");
    using RT = typename funk::type_support::function_traits<F>::return_type;
    using CT = C<std::function<RT(void)>>;
    CT out {};
    auto const ends = std::make_tuple (std::forward<C<Ts>>(cs).end()...);
    for (auto iter = std::make_tuple (std::forward<C<Ts>>(cs).begin()...);
        !funk::utils::tuple_any_equal(iter, ends);
         funk::utils::tuple_increment(iter))
    {
        funk::type_support::container_traits<CT>::insert
            (out, funk::apply_tuple
                (f, funk::utils::tuple_dereference(iter)));
    }
    return out; 
}
*/
/*
template <template <class...> class C, class ... Ts>
static constexpr decltype(auto) unzip (C<std::tuple<Ts...>> & v)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not unzippable");
    using VT = typename std::tuple<Ts...>;
    return detail::unzip_helper (v, typename utils::seq_gen<std::tuple_size<VT>::value>::type());
}
*/
template <template <class...> class C, class ... Ts>
static constexpr decltype(auto) unzip (C<std::tuple<Ts...>> && v)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not unzippable");
    using VT = typename std::tuple<Ts...>;
    return detail::unzip_helper (std::forward(v), typename utils::seq_gen<std::tuple_size<VT>::value>::type());
}
/*
template <template <class...> class C, class ... Ts>
static constexpr decltype(auto) unzip (C<std::tuple<Ts...>> const& v)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not unzippable");
    using VT = typename std::tuple<Ts...> const;
    return detail::unzip_helper (v, typename utils::seq_gen<std::tuple_size<VT>::value>::type());
}

template <template <class...> class C, class ... Ts>
static constexpr decltype(auto) unzip (C<std::tuple<Ts...>> const&& v)
{
    static_assert
        (funk::utils::all_true((funk::is_zippable<C, Ts>::value)...), "container type not unzippable");
    using VT = typename std::tuple<Ts...> const;
    return detail::unzip_helper (std::forward(v), typename utils::seq_gen<std::tuple_size<VT>::value>::type());
}
*/
auto v1 = std::vector<int> { 1, 2, 3 };
auto v2 = std::vector<float> { 1.0, 2.0, 3.0 };
auto v3 = std::vector<double> { 1.0, 2.0, 3.0 };

auto r1 = funk::zip (v1, v2, v3);
//auto r2 = funk::zip_with ([](int a, float b, double c) { return a + b + c; }, v1, v2, v3);
//auto r3 = funk::fold (r2);

} // namesapce funk

#endif // ifndef ZIP_HPP

