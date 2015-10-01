//
// Parser combinators
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef COMBINATORS_HPP
#define COMBINATORS_HPP

#include <type_traits>

#include "range.hpp"
#include "parser.hpp"

#include "funktional/include/type_support/function_traits.hpp"

namespace rpc
{
namespace core
{
    template <typename It, typename V, typename F,
        typename = std::enable_if_t<is_parser_instance<typename fnk::type_support::function_traits<F>::result_type>::value>>
    inline decltype(auto) bind (parser<It, V> const& p, F && f)
    {
        using R = typename fnk::type_support::function_traits<F>::result_type;
        return parser<It, typename R::value_type>
        {
            .parse = [=](typename parser_traits<R>::range_type const& r)
            {
                return fnk::concat
                    (fnk::map
                        ([=](typename parser<It, V>::result_type const& e)
                         {
                            if (parser<It, V>::is_value_result (e))
                                return fnk::eval
                                    (fnk::eval(f, parser<It, V>::result_value (e)).parse, parser<It, V>::result_range (e));
                            else
                                return std::list<typename parser_traits<R>::result_type>{ failure{} };
                         },
                        fnk::eval (p.parse, r)));
            }
        }; 
    }

    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename =
            std::enable_if_t<std::is_same<typename parser_traits<P>::value_type, typename parser_traits<Q>::value_type>::value>,
        typename =
            std::enable_if_t<std::is_same<typename parser_traits<P>::token_type, typename parser_traits<Q>::token_type>::value>>
    inline decltype(auto) combine (P && p, Q && q)
    {
        using It = typename parser_traits<P>::range_type::iter_type;
        using V  = typename parser_traits<P>::value_type;
        
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                return fnk::append (fnk::eval (p.parse, r), fnk::eval (q.parse, r));
            }
        };
    }

    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename = 
            std::enable_if_t<std::is_same<typename parser_traits<P>::range_type, typename parser_traits<Q>::range_type>::value>,
        typename = 
            std::enable_if_t<std::is_same<typename parser_traits<P>::value_type, typename parser_traits<Q>::token_type>::value>>
    inline decltype(auto) sequence (P && p, Q && q)
    {
        using PIt = typename parser_traits<P>::range_type::iter_type;
        using PV = typename parser_traits<P>::value_type;
        using QV = typename parser_traits<Q>::value_type;
        using QR = typename parser_traits<Q>::result_type;
        return parser<PIt, QV>
        {
            .parse = [=](typename parser<PIt, QV>::range_type const& r)
            {
                auto l1 = fnk::eval (p.parse, r);
                std::list<std::list<QR>> preout;
                for (auto const& e : l1)
                {
                    if (parser<PIt, PV>::is_result (e))
                        fnk::type_support::container_traits<std::list<std::list<QR>>>::insert
                            (preout, fnk::eval (q.parse, parser<PIt, PV>::result_range (e)));
                    else
                        fnk::type_support::container_traits<std::list<std::list<QR>>>::insert
                            (preout, std::list<QR>{failure{parser<PIt, PV>::failure_message(e)}});
                }
                return fnk::concat (preout);
            }
        };
    }
    
    template <typename P, typename ... Qs, typename = std::enable_if_t<sizeof...(Qs) >= 2>>
    inline decltype(auto) sequence (P && p, Qs && ... qs)
    {
        return sequence (p, sequence (qs...));    
    }

    //
    // Sequence two parsers, but ignore the result of the left argument.
    // Note that the value types do not have to be the same; in fact, the parser
    // returned has the value type of the right argument.
    //
    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename = std::enable_if_t
            <std::is_same<typename parser_traits<P>::token_type, typename parser_traits<Q>::token_type>::value>>
    inline decltype(auto) ignorel (P && p, Q && q)
    {
        using PIt = typename parser_traits<P>::range_type::iter_type;
        using PV = typename parser_traits<P>::value_type;
        using QV = typename parser_traits<Q>::value_type;
        using QR = typename parser_traits<Q>::result_type;
        return parser<PIt, QV>
        {
            .parse = [=](typename parser<PIt, QV>::range_type const& r)
            {
                auto l1 = fnk::eval (p.parse, r);
                if (parser<PIt, PV>::is_failure (l1.front()))
                    return std::list<QR> { failure{} };
                else 
                    return fnk::eval (q.parse, parser<PIt, PV>::result_range (l1.back()));
            }
        };
    }

    //
    // Sequence two parsers, but ignore the result of the right argument
    // Note that the value types do not have to be the same; in fact, the parser
    // returned has the value type of the left argument.
    //
    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename = std::enable_if_t
            <std::is_same<typename parser_traits<P>::value_type, typename parser_traits<Q>::token_type>::value>>
    inline decltype(auto) ignorer (P && p, Q && q)
    {
        using PIt = typename parser_traits<P>::range_type::iter_type;
        using PV = typename parser_traits<P>::value_type;
        using PR = typename parser_traits<P>::result_type;
        using QIt = typename parser_traits<Q>::range_type::iter_type;
        using QV = typename parser_traits<Q>::value_type;
        return parser<PIt, PV>
        {
            .parse = [=](typename parser<PIt, PV>::range_type const& r)
            {
                auto l1 = fnk::eval (p.parse, r);
                if (parser<PIt, PV>::is_failure (l1.front()))
                    return std::list<PR> { failure{} };
                else {
                    auto l2 = fnk::eval (q.parse, parser<PIt, PV>::result_range (l1.back()));
                    if (parser<QIt, QV>::is_failure (l2.front()))
                        return std::list<PR> { failure{} };
                    else {
                        auto l1_back = l1.back();
                        l1.pop_back();
                        l1.push_back
                            (std::make_pair
                                (parser<PIt, PV>::result_value (l1_back), parser<QIt, QV>::result_range (l2.back())));
                        return l1;
                    }
                }
            }
        };
    }
    
    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename =
            std::enable_if_t<std::is_same<typename parser_traits<P>::value_type, typename parser_traits<Q>::value_type>::value>,
        typename =
            std::enable_if_t<std::is_same<typename parser_traits<P>::token_type, typename parser_traits<Q>::token_type>::value>>
    inline decltype(auto) option (P && p, Q && q)
    {
        using It = typename parser_traits<P>::range_type::iter_type;
        using V  = typename parser_traits<P>::value_type;
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l = fnk::eval (p.parse, r);
                return parser<It, V>::is_failure (l.front()) ? fnk::eval (q.parse, r) : l;
            }
        };
    }
    
    template <typename P, typename ... Qs, typename = std::enable_if_t<sizeof...(Qs) >= 2>,
        typename = std::enable_if_t<is_parser_instance<P>::value>>
    inline decltype(auto) option (P && p, Qs && ... qs)
    {
        return option (p, option (qs...));
    }

    //
    // One or more successful parses
    //
    template <typename It, typename V>
    inline decltype(auto) some (parser<It, V> const& p)
    {
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l1 = eval (p.parse, r);
                if (parser<It, V>::is_failure (l1.front()))
                    return std::list<typename parser<It, V>::result_type>{ failure{"expected at least one result"} };
                else {
                    while (true)
                    {
                        auto l2 = eval (p.parse, parser<It, V>::result_range(l1.back()));
                        if (parser<It, V>::is_failure (l2.front()))
                            return l1;
                        else
                            l1.splice (l1.cend(), l2);
                    }
                } 
            }
        }; 
    }

    //
    // Exactly n successful parses
    //
    template <typename It, typename V>
    inline decltype(auto) some (parser<It, V> const& p, std::size_t const n)
    {
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l1 = eval (p.parse, r);
                if (parser<It, V>::is_failure (l1.front()))
                    return std::list<typename parser<It, V>::result_type>{ failure{"expected at least one result"} };
                else {
                    for (std::size_t i = 1; i < n; ++i) // run n-1 more times
                    {
                        auto l2 = eval (p.parse, parser<It, V>::result_range(l1.back()));
                        if (parser<It, V>::is_failure (l2.front()))
                            return l1;
                        else
                            l1.splice (l1.cend(), l2);
                    }
                    return l1;
                } 
            }
        }; 
    }

    //
    // Zero or more successful parses
    //
    template <typename It, typename V>
    inline decltype(auto) many (parser<It, V> const& p)
    {
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l1 = eval (p.parse, r);
                if (parser<It, V>::is_failure (l1.front()))
                    return std::list<typename parser<It, V>::result_type>{ std::make_pair (empty_result<V>{}, r) };
                else {
                    while (true)
                    {
                        auto l2 = eval (p.parse, parser<It, V>::result_range(l1.back()));
                        if (parser<It, V>::is_failure (l2.front()))
                            return l1;
                        else
                            l1.splice (l1.cend(), l2);
                    }
                }
            }
        }; 
    }

    //
    // Zero or more successful parses, but at most n successes
    //
    template <typename It, typename V>
    inline decltype(auto) manyto (parser<It, V> const& p, std::size_t const n)
    {
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l1 = eval (p.parse, r);
                if (parser<It, V>::is_failure (l1.front()))
                    return std::list<typename parser<It, V>::result_type>{ std::make_pair (empty_result<V>{}, r) };
                else {
                    for (std::size_t i = 1; i < n; ++i) // run n-1 more times
                    {
                        auto l2 = eval (p.parse, parser<It, V>::result_range(l1.back()));
                        if (parser<It, V>::is_failure (l2.front()))
                            return l1;
                        else
                            l1.splice (l1.cend(), l2);
                    }
                    if (parser<It,V>::is_result (eval (p.parse, parser<It, V>::result_range(l1.back())))) // check that n+1 runs fails
                        return std::list<typename parser<It, V>::result_type>
                            { failure {"expected at most " + std::to_string(n) + " matches"} };
                    else
                        return l1;
                }
            }
        }; 
    }

    //
    // Either 0 or 1 successful parses.
    //
    template <typename It, typename V>
    inline decltype(auto) optional (parser<It, V> const& p)
    {
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l1 = eval (p.parse, r);
                if (parser<It, V>::is_result (l1.front()))
                    return l1;
                else
                    return std::list<typename parser<It, V>::result_type>{ std::make_pair (empty_result<V>{}, r) };
            }
        };
    }
} // namespace core
} // namespace rpc

#endif // ifndef COMBINATORS_HPP

