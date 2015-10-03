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
#include "../basic/atom_parsers.hpp"

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
        typename = std::enable_if_t
            <std::is_same<typename parser_traits<P>::value_type, typename parser_traits<Q>::value_type>::value>,
        typename = std::enable_if_t
            <std::is_same<typename parser_traits<P>::range_type, typename parser_traits<Q>::range_type>::value>>
    inline decltype(auto) sequence (P && p, Q && q)
    {
        using It = typename parser_traits<P>::range_type::iter_type;
        using V = typename parser_traits<P>::value_type;
        using R = typename parser_traits<P>::result_type;
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l = fnk::eval (p.parse, r);
                std::list<std::list<R>> out {l};
                for (auto const& e : l)
                {
                    if (parser<It, V>::is_result (e))
                        fnk::type_support::container_traits<std::list<std::list<R>>>::insert
                            (out, fnk::eval (q.parse, parser<It, V>::result_range (e)));
                    else
                        fnk::type_support::container_traits<std::list<std::list<R>>>::insert
                            (out, std::list<R>{failure{parser<It, V>::failure_message(e)}});
                }
                return fnk::concat (out);
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
            <std::is_same<typename parser_traits<P>::range_type, typename parser_traits<Q>::range_type>::value>>
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
            <std::is_same<typename parser_traits<P>::range_type, typename parser_traits<Q>::range_type>::value>>
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
            std::enable_if_t<std::is_same<typename parser_traits<P>::range_type, typename parser_traits<Q>::range_type>::value>>
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
    // Either 0 or 1 successful parses. If the primary parse fails,
    // then the default value is returned; hence, this parser ALWAYS
    // succeeds, but may or may not consume input.
    //
    template <typename It, typename V>
    inline decltype(auto) optional (parser<It, V> const& p, V && default_value)
    {
        return option (p, basic::unit<It> (default_value));
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
    inline decltype(auto) many (parser<It, V> const& p, std::size_t const n)
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
    // Reduction parser: reduces over a list of parse values (using a foldl) to produce
    // a final parse result.
    //
    template <typename It, typename V,
        typename = std::enable_if_t<fnk::monoid<V>::is_monoid_instance::value>>
    inline decltype(auto) reduce (parser<It, V> const& p)
    {
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l = fnk::eval (p.parse, r);
                if (parser<It, V>::is_parse_success (l))
                    return std::list<typename parser<It, V>::result_type>
                        { std::make_pair
                            (fnk::fold (parser<It, V>::values (l)),
                             parser<It, V>::result_range (l.back())) };
                else
                    return std::list<typename parser<It, V>::result_type> { failure {parser<It, V>::failure_message (l.front())} };
            }
        };
    }  
    
    //
    // Reduction parser: reduces over a list of parse values (using a foldr) to produce
    // a final parse result.
    //
    template <typename F, typename B, typename It, typename V,
        typename = std::enable_if_t<fnk::monoid<V>::is_monoid_instance::value>>
    inline decltype(auto) reduce (parser<It, V> const& p, F && f, B && b)
    {
        using W = decltype(fnk::fold<std::list<typename parser<It, V>::value_type>>(std::list<typename parser<It, V>::value_type>()));
        return parser<It, W>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
                auto l = fnk::eval (p.parse, r);
                if (parser<It, V>::is_parse_success (l))
                    return std::list<typename parser<It, W>::result_type>
                        { std::make_pair
                            (fnk::foldr (f, std::forward<B>(b), parser<It, V>::values (l)),
                             parser<It, V>::result_range (l.back())) };
                else
                    return std::list<typename parser<It, W>::result_type> { failure {parser<It, V>::failure_message (l.front())} };
            }
        };
    }

    //
    // Lift a parser to a new value type using an existing type conversion.
    //
    template <typename U, typename It, typename V>
    inline decltype(auto) lift (parser<It, V> const& p)
    {
        return fnk::functor<parser<It, V>>::fmap
            ([](V const& v) { return static_cast<U>(v); }, p);
    }     

    //
    // Lift a parser to a new value type using an explicitly provided function,
    // which is the same as fmap.
    //
    template <typename F, typename It, typename V>
    inline decltype(auto) lift (parser<It, V> const& p, F && f)
    {
        return fnk::functor<parser<It, V>>::fmap (f, p);
    }

    //
    // Lift a parser to a new value type using an existing type conversion,
    // then fold over the results to a reduced final parse value.
    //
    template <typename U, typename It, typename V,
        typename = std::enable_if_t<fnk::monoid<U>::is_monoid_instance::value>>
    inline decltype(auto) liftreduce (parser<It, V> const& p)
    {
        return reduce (lift<U>(p));
    }

    //
    // Lift a parser to a new value type using an explicitly provided function,
    // then fold over the results to reduce to a final parse value.
    //
    template <typename F, typename B, typename It, typename V,
        typename = std::enable_if_t
            <fnk::monoid<typename fnk::type_support::function_traits<F>::return_type>::is_monoid_instance::value>>
    inline decltype(auto) liftreduce (parser<It, V> const& p, F && f)
    {
        return reduce (lift (p, f));
    }
    
    //
    // Lift a parser to a new value type using an explicitly provided function,
    // then foldr over the results using the second provided function to reduce to a final parse value.
    //
    template <typename F, typename G, typename B, typename It, typename V,
        typename = std::enable_if_t
            <fnk::monoid<typename fnk::type_support::function_traits<F>::return_type>::is_monoid_instance::value>>
    inline decltype(auto) liftreduce (parser<It, V> const& p, F && f, G && g, B && b)
    {
        return reduce (lift (p, f), g, b);
    }
} // namespace core
} // namespace rpc

#endif // ifndef COMBINATORS_HPP

