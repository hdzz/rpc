//
// Some primitive parsers
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ATOM_PARSERS_HPP
#define ATOM_PARSERS_HPP

#include <type_traits>
#include <iterator>
#include <list>
#include <vector>

#include "core/range.hpp"
#include "core/parser.hpp"

#include "funktional/include/type_support/container_traits.hpp"
#include "funktional/include/type_support/function_traits.hpp"
#include "funktional/include/utility/bool_utils.hpp"
#include "funktional/include/utility/type_utils.hpp"

namespace rpc
{
namespace basic
{
    template <typename It, typename V, typename R = core::range<It>>
    auto const fail = core::parser<It, V, R>
    {
        .description = "[failure]",
        .parse = [](typename core::parser<It, V, R>::accumulator_type & acc)
        {
            acc.insert
                (core::parse_result<V> {core::failure {"[failure]"}},
                torange (acc));
            return acc;
        }
    };

    template <typename It, typename V, typename R = core::range<It>>
    inline decltype(auto) failwith (std::string const& description)
    {
        return core::override_description (fail<It, V, R>, description);
    }
    
    template <typename It, typename V, typename R = core::range<It>>
    auto const pass = core::parser<It, V, R>
    {
        .description = "[pass]",
        .parse = [](typename core::parser<It, V, R>::accumulator_type & acc)
        {
            return acc;
        }
    };
 
    template <typename It, typename V, typename R = core::range<It>>
    inline decltype(auto) unit (V const& v)
    {
        using U = std::decay_t<V>;

        return core::parser<It, U, R>
        {
            .description =
                "[pure: " +
                fnk::utility::to_string<V> (v) +
                " :: " +
                fnk::utility::type_name<V>::name() +
                "]",
            .parse =
            [=](typename core::parser<It, U, R>::accumulator_type & acc)
            {
                acc.insert
                    (core::parse_result<U> {static_cast<U> (v)}, torange (acc));
                return acc;
            }
        };
    }
 
    template <typename It, typename V, typename R = core::range<It>>
    auto item = core::parser<It, V, R>
    {
        .description = "[item :: " + fnk::utility::type_name<V>::name() + "]",
        .parse = [](typename core::parser<It, V, R>::accumulator_type & acc)
        {
            if (acc.range_empty()) {
                acc.insert
                    (core::failure {"expected [item :: " +
                                   fnk::utility::type_name<V>::name() +
                                   "]"},
                    torange (acc));
                return acc;
            } else {
                acc.insert
                    (core::parse_result<V>
                        {static_cast<V> (torange_head (acc))},
                    torange_tail (acc));
                return acc;
            }
        }
    };

    template <typename It, typename T, typename R, typename Pr, 
        typename = std::enable_if_t
            <std::is_same
                <typename fnk::type_support::function_traits<Pr>::return_type,
                bool>::value>,
        typename = std::enable_if_t
            <std::is_convertible
                <typename fnk::type_support::function_traits<Pr>
                    ::template argument<0>::type, 
                typename std::iterator_traits<It>::value_type>::value>>
    inline decltype(auto) satisfy (Pr && predicate, std::string const& dsc = "")
    {
        return core::parser<It, T, R>
        {
            .description = "['" + dsc + "']",
            .parse =
            [=](typename core::parser<It, T, R>::accumulator_type & acc)
            {
                if (acc.range_empty()) {
                    acc.insert
                        (core::failure {"expected [item :: " +
                                       fnk::utility::type_name<T>::name() +
                                       "]"},
                        torange (acc));
                    return acc;
                } else if (fnk::eval (predicate, torange_head (acc))) {
                    acc.insert
                        (core::parse_result<T>
                            {static_cast<T> (torange_head (acc))},
                        torange_tail (acc));
                    return acc;
                } else {
                    acc.insert
                        (core::failure {"expected ['" +
                                       dsc +
                                       "']"},
                        torange (acc));
                    return acc;
                }
            }
        };
    }

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline decltype(auto) token (T && t)
    {
        auto pred ([t_ = std::forward<T>(t)] (T const& e) { return t_ == e; }); 
        return satisfy<It, T, R>
            (pred,
            "pure: " +
            fnk::utility::to_string<T>(t) +
            " :: " +
            fnk::utility::type_name<T>::name());
    }

namespace detail
{
    template <typename L>
    std::string list_to_string (L const& l)
    {
        using T = typename L::value_type;
        if (l.size() == 0)
            return "[]";
        else {
            std::string res ("[");
            auto it = l.begin();
            auto pend = std::prev (l.end());
            for (; it != pend; ++it) {
                res += fnk::utility::to_string<T> (*it);
                res += ", ";
            }
            res += fnk::utility::to_string<T> (*it);
            res += "]";
            return res;
        }
    }
} // namespace detail

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline decltype(auto) one_of (std::initializer_list<T> l)
    {
        // Waiting for std::initializer_list to have constexpr .size() method,
        // then we can remove std::vector and have a statically created
        // std::array.

        auto pred
            ([lc = std::vector<T>(l)](T const& t) 
            {
                for (auto const& e : lc)
                    if (t == e)
                        return true;
                return false;
            });
        return satisfy<It, T, R>
            (pred,
            "pure: (one of) " +
            detail::list_to_string (l) +
            " :: " +
            fnk::utility::type_name<T>::name());
    }
    
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline decltype(auto) none_of (std::initializer_list<T> l)
    {
        // Waiting for std::initializer_list to have constexpr .size() method,
        // then we can remove std::vector and have a statically created
        // std::array.
        
        auto pred
            ([lc = std::vector<T>(l)](T const& t) 
             { 
                bool is_okay = true;
                for (auto const& e : lc)
                    is_okay = is_okay && (t != e);
                return is_okay;
            });
        return satisfy<It, T, R>
            (pred,
            "pure: (none of) " +
            detail::list_to_string (l) +
            " :: " +
            fnk::utility::type_name<T>::name());
    }

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>,
             typename =
                std::enable_if_t
                    <fnk::utility::is_well_formed
                        <decltype(T::operator<=), T const&, T const&>::value>>
    inline decltype(auto) in_range (T const& start, T const& end)
    {
        auto pred ([=](T const& t) { return start <= t && t <= end; });
        return satisfy<It, T, R>
            (pred,
             "pure: (in) ["
             + fnk::utility::to_string<T>(start)
             + ", "
             + fnk::utility::to_string<T>(end)
             + "] :: "
             + fnk::utility::type_name<T>::name());
    }
} // namespace basic
} // namespace rpc

#endif // ifndef ATOM_PARSERS_HPP

