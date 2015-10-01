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
        }
    };

    template <typename It, typename V = It>
    inline decltype(auto) unit (V && v)
    {
        return core::parser<It, V>
        {
            .parse = [=](typename core::parser<It, V>::range_type const& r)
            {
                return std::list<typename core::parser<It, V>::result_type> { std::make_pair (std::forward<V>(v), r) };
            }
        };
    }
 
    template <typename It, typename V>
    auto item = core::parser<It, V>
    {
        .parse = [](typename core::parser<It, V>::range_type const& r)
        {
            return r.is_empty() ? std::list<typename core::parser<It, V>::result_type>{ core::failure{"expected token to parse"} }
                                : std::list<typename core::parser<It, V>::result_type>{ std::make_pair (r.head(), r.tail()) }; 
        }
    };

    template <typename It, typename P,
        typename = std::enable_if_t<std::is_same<typename fnk::type_support::function_traits<P>::return_type, bool>::value>,
        typename = std::enable_if_t
            <std::is_convertible<typename fnk::type_support::function_traits<P>::template argument<0>::type, 
                                 typename std::iterator_traits<It>::value_type>::value>>
    inline constexpr decltype(auto) satisfy (P && p, std::string const& description = std::string(""))
    {
        using T = std::decay_t<typename fnk::type_support::function_traits<P>::template argument<0>::type>;
        using OT = std::list<typename core::parser<It, T>::result_type>;
        
        return core::parser<It, T>
        {
            .parse = [=](typename core::parser<It, T>::range_type const& r)
            {
                if (r.is_empty())
                    return OT { core::failure{"no remaining tokens to parse"} };
                else {
                    if (fnk::eval (p, r.head()))
                        return OT { std::make_pair (r.head(), r.tail()) };
                    else
                        return OT { core::failure{std::string("predicate '") + description + std::string("' failed")} };
                }
            } 
        };
    }

    template <typename It, typename V>
    inline constexpr decltype(auto) token (V && v)
    {
        return satisfy<It> ([v_ = std::forward<V>(v)](V const& e) { return v_ == e; });
    }

    template <typename It, typename V>
    inline constexpr decltype(auto) one_of (std::initializer_list<V> l)
    {
        return satisfy<It>
            ([lc = std::vector<V>(l)](V const& v)
            {
                for (auto const& e : lc)
                    if (v == e)
                        return true;
                return false;
            });
    }
} // namespace basic
} // namespace rpc

#endif // ifndef ATOM_PARSERS_HPP

