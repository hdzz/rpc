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
    template <typename It, typename V>
    static auto fail = core::parser<It, V>
    {
        .parse = [](typename core::parser<It, V>::range_type const& /*r*/)
        {
            return std::list<typename core::parser<It, V>::result_type> { core::failure{} };
        },
        .description = "[failure]"
    };

    template <typename It, typename V>
    inline decltype(auto) unit (V && v)
    {
        using U = std::remove_reference_t<V>;

        return core::parser<It, U>
        {
            .parse = [=](typename core::parser<It, U>::range_type const& r)
            {
                return std::list<typename core::parser<It, U>::result_type> { std::make_pair (v, r) };
            },
            .description
                = std::string("[pure: ")
                + fnk::utility::to_string<V> (v)
                + std::string(" :: ")
                + fnk::utility::type_name<V>::name()
                + std::string("]")
        };
    }
 
    template <typename It, typename V>
    auto item = core::parser<It, V>
    {
        .parse = [](typename core::parser<It, V>::range_type const& r)
        {
            return r.is_empty() ? std::list<typename core::parser<It, V>::result_type>
                                        { core::failure
                                            { std::string("expected")
                                            + std::string("[item :: ")
                                            + fnk::utility::type_name<V>::name()
                                            + std::string("]") }
                                        }
                                : std::list<typename core::parser<It, V>::result_type>{ std::make_pair (r.head(), r.tail()) }; 
        },
        .description = std::string("[item :: ") + fnk::utility::type_name<V>::name() + std::string("]")
    };

    template <typename It, typename P,
        typename = std::enable_if_t<std::is_same<typename fnk::type_support::function_traits<P>::return_type, bool>::value>,
        typename = std::enable_if_t
            <std::is_convertible<typename fnk::type_support::function_traits<P>::template argument<0>::type, 
                                 typename std::iterator_traits<It>::value_type>::value>>
    inline decltype(auto) satisfy (P && p, std::string const& description = std::string(""))
    {
        using T = std::decay_t<typename fnk::type_support::function_traits<P>::template argument<0>::type>;
        using OT = std::list<typename core::parser<It, T>::result_type>;
        
        return core::parser<It, T, T>
        {
            .parse = [=](typename core::parser<It, T>::range_type const& r)
            {
                if (r.is_empty())
                    return OT
                        { core::failure
                            { std::string("expected")
                            + std::string("[item :: ")
                            + fnk::utility::type_name<T>::name()
                            + std::string("]") }
                        };
                else {
                    if (fnk::eval (p, r.head()))
                        return OT { std::make_pair (r.head(), r.tail()) };
                    else
                        return OT { core::failure{std::string("expected [") + description + std::string("]")} };
                }
            },
            .description = std::string("[") + description + std::string("]")
        };
    }

    template <typename It, typename T = typename std::iterator_traits<It>::value_type>
    inline decltype(auto) token (T && t)
    {
        return satisfy<It>
            ([t_ = std::forward<T>(t)](T const& e) { return t_ == e; },
             std::string("pure: ") + fnk::utility::to_string<T>(t) + std::string(" :: ") + fnk::utility::type_name<T>::name());
    }

    template <typename It, typename T = typename std::iterator_traits<It>::value_type>
    inline decltype(auto) one_of (std::initializer_list<T> l)
    {
        std::string valnames;
        auto i = l.size();
        for (auto const& e : l)
        {
            if (--i != 0) valnames.append (fnk::utility::to_string<T>(e) + std::string(", "));
            else          valnames.append (fnk::utility::to_string<T>(e));
        }
        
        return satisfy<It>
            ([lc = std::vector<T>(l)](T const& t) // Waiting for std::initializer_list to have constexpr .size() method,
            {                                     // then we can remove std::vector and have a statically created std::array.
                for (auto const& e : lc)
                    if (t == e)
                        return true;
                return false;
            },
            std::string("pure: (in) {") + valnames + std::string("} :: ") + fnk::utility::type_name<T>::name());
    }
    
    template <typename It, typename T = typename std::iterator_traits<It>::value_type>
    inline decltype(auto) none_of (std::initializer_list<T> l)
    {
        std::string valnames;
        auto i = l.size();
        for (auto const& e : l)
        {
            if (--i != 0) valnames.append (fnk::utility::to_string<T>(e) + std::string(", "));
            else          valnames.append (fnk::utility::to_string<T>(e));
        }

        return satisfy<It>
            ([lc = std::vector<T>(l)](T const& t) // Waiting for std::initializer_list to have constexpr .size() method,
            {                                     // then we can remove std::vector and have a statically created std::array.
                bool is_okay = true;
                for (auto const& e : lc)
                    is_okay = is_okay && (t != e);
                return is_okay;
            },
            std::string("pure: (not) {") + valnames + std::string("} :: ") + fnk::utility::type_name<T>::name());
    }

    //
    // Checks whether a token `t` is in the range [`start, `end`].
    //
    template <typename It, typename T = typename std::iterator_traits<It>::value_type,
        typename = std::enable_if_t<fnk::utility::is_well_formed<decltype(T::operator<=), T const&, T const&>::value>>
    inline decltype(auto) in_range (T const& start, T const& end)
    {
        return satisfy<It>
            ([=](T const& t) { return start <= t && t <= end; },
             std::string("pure: (in) [")
             + fnk::utility::to_string<T>(start)
             + std::string(", ")
             + fnk::utility::to_string<T>(end)
             + std::string("] :: "),
             + fnk::utility::type_name<T>::name());
    }
} // namespace basic
} // namespace rpc

#endif // ifndef ATOM_PARSERS_HPP

