//
// Parser combinators
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef COMBINATORS_HPP
#define COMBINATORS_HPP

#include <iostream>
#include <type_traits>

#include "range.hpp"
#include "parser.hpp"
#include "../basic/atom_parsers.hpp"

#include "funktional/include/compose.hpp"
#include "funktional/include/type_support/function_traits.hpp"

namespace rpc
{
namespace core
{
    template <typename F, typename It, typename V, typename R>
    decltype(auto) lift (parser<It, V, R> const&, F &&);


    template <typename It, typename V, typename R, typename F>
    inline decltype(auto) bind (parser<It, V, R> const& p, F && f)
    {
        using Ftraits = fnk::type_support::function_traits<F>;
        using Fret = typename Ftraits::return_type;
        
        static_assert (is_parser_instance<Fret>::value,
                      "function being bound must return a parser.");

        using Qtraits = parser_traits<Fret>;
        using Qv = typename Qtraits::value_type;
       
        static_assert (std::is_same<V, Qv>::value,
                      "value types of bound parsers must agree.");

        return parser<It, Qv, R>
        {
            .description = 
                "[" + 
                p.description + 
                " //bind// " + 
                fnk::utility::format_function_type<F>() + 
                "]",
            .parse = [=](typename Qtraits::accumulator_type & acc)
            {
                auto res (p.parse (acc));
                if (parse_success (res)) {
                    auto q (fnk::eval (f, toresult_value (acc)));
                    return q.parse (res);
                } else {
                    return res;
                }
            }
        }; 
    }

    //
    // bind even on failure.
    //
    template <typename It, typename V, typename R, typename F>
    inline decltype(auto) bindf (parser<It, V, R> const& p, F && f)
    {
        using Fret =
            typename fnk::type_support::function_traits<F>::return_type;

        static_assert (is_parser_instance<Fret>::value,
                      "function being bound must return a parser.");

        using Qtraits = parser_traits<Fret>;
        using Qv = typename Qtraits::value_type;
       
        static_assert (std::is_same<V, Qv>::value,
                      "value types of bound parsers must agree.");

        return parser<It, Qv, R>
        {
            .description = 
                "[" + 
                p.description + 
                " //bindf// " + 
                fnk::utility::format_function_type<F>() + 
                "]",
            .parse = [=](typename Qtraits::accumulator_type & acc)
            {
                auto res (p.parse (acc));
                auto q (fnk::eval (f, toresult_value (res)));
                return q.parse (res);
            }
        }; 
    }
 
    template <typename It, typename V, typename R>
    inline decltype(auto) ignore (parser<It, V, R> const& p)
    {
        return override_description
            (lift
                (p, [](V const& /*v*/) { return empty<V>{}; }), p.description);
    }

    template <typename U, typename It, typename V, typename R>
    inline decltype(auto) liftignore (parser<It, V, R> const& p)
    {
        return parser<It, U, R>
        {
            .description = p.description,
            .parse = [=](typename parser<It, U, R>::accumulator_type & acc)
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (acc) };
                 
                auto res (p.parse (mock));
                if (parse_success (res)) {
                    acc.insert (parse_result<U> {empty<U> {}}, torange (mock));
                    return acc;
                } else {
                    acc.insert
                        (failure {toresult_failure (mock)}, torange (mock));
                    return acc;
                }
            }
        };
    }

    template <typename It, typename V, typename R>
    inline decltype(auto) branch (parser<It, V, R> const& p,
                                  parser<It, V, R> const& succ,
                                  parser<It, V, R> const& next)
    {
        return override_description
            (bindf
                (p,
                [=](typename parser<It, V, R>::result_type const& res)
                {
                    return is_success (res) ? succ : next;
                }),
            "[(branch) :: " +
            p.description + 
            " => " + 
            succ.description +
            " | " + 
            next.description);
    }
 
    template <typename It, typename V, typename R = range<It>>
    inline decltype(auto) sequence
        (parser<It, V, R> const& p, parser<It, V, R> const& q)
    {
        return override_description
            (branch (p, q, basic::failwith<It, V, R> (p.description)),
             "[" + p.description + " //then// " + q.description + "]");
    }

    template <typename P,
             typename ... Qs,
             typename = std::enable_if_t<sizeof...(Qs) >= 2>>
    inline decltype(auto) sequence (P && p, Qs && ... qs)
    {
        return sequence (p, sequence (qs...));    
    }

    //
    // Sequence two parsers, but ignore the result of the left argument.
    // Note that the value types do not have to be the same; in fact, the parser
    // returned has the value type of the right argument.
    //
    template <typename P, typename Q>
    inline decltype(auto) ignorel (P && p, Q && q)
    {
        using U = typename parser_traits<Q>::value_type;
        return sequence (liftignore<U> (p), q);
    }

    //
    // Sequence two parsers, but ignore the result of the right argument
    // Note that the value types do not have to be the same; in fact, the parser
    // returned has the value type of the left argument.
    //
    template <typename P, typename Q>
    inline decltype(auto) ignorer (P && p, Q && q)
    {
        using U = typename parser_traits<P>::value_type;
        return sequence (p, liftignore<U> (q));
    }
 
    template <typename P, typename Q>
    inline decltype(auto) option (P && p, Q && q)
    {
        using It = typename parser_traits<P>::iter_type;
        using V  = typename parser_traits<P>::value_type;
        using R  = typename parser_traits<P>::range_type;
 
        return override_description
            (branch
                (p, basic::pass<It, V, R>, q),
            "[" + p.description + " //or// " + q.description + "]");
    }
 
    template <typename P,
             typename ... Qs,
             typename = std::enable_if_t<sizeof...(Qs) >= 2>,
             typename = std::enable_if_t<is_parser_instance<P>::value>>
    inline decltype(auto) option (P && p, Qs && ... qs)
    {
        return option (p, option (qs...));
    }

    //
    // Either 0 or 1 successful parses. If the primary parse fails,
    // then pass silently.
    //
    template <typename It, typename V, typename R>
    inline decltype(auto) optional (parser<It, V, R> const& p)
    {
        return override_description
            (option (p, basic::pass<It, V, R>), "(optional) " + p.description);
    }

    //
    // Either 0 or 1 successful parses. If the primary parse fails,
    // then the default value is returned; hence, this parser ALWAYS
    // succeeds, but may or may not consume input.
    //
    template <typename It, typename V, typename R>
    inline decltype(auto) optional
        (parser<It, V, R> const& p, V const& default_value)
    {
        auto dflt (basic::unit<It, V, R> (default_value));
        return override_description
            (option (p, dflt),
            "(optional) " + p.description + " | " + dflt.description);
    }

    //
    // At least one but at most n successful parses; if n == 0,
    // then there is no upper bound.
    //
    template <typename It, typename V, typename R>
    inline decltype(auto) some
        (parser<It, V, R> const& p, std::size_t const n = 0)
    {
        return override_description
            (branch
                (p,
                fnk::iterate_while (p, parse_success, n == 0 ? 0 : n - 1),
                basic::failwith<It, V, R> (p.description)),
            "[(some) " + p.description + "]");
    }

    //
    // Zero or more successful parses
    //
    template <typename It, typename V, typename R>
    inline decltype(auto) many (parser<It, V, R> const& p)
    {
        return override_description
            (branch
                (p,
                fnk::iterate_while (p, parse_success), basic::pass<It, V, R>),
            "[(many) " + p.description + "]");
    }

    //
    // Reduction parser: reduces over a list of parse values (using a foldl)
    // to produce a final parse result.
    //
    template <typename It, typename V, typename R,
        typename = std::enable_if_t<fnk::monoid<V>::is_monoid_instance::value>>
    inline decltype(auto) reduce (parser<It, V, R> const& p)
    {
        static_assert (fnk::monoid<V>::is_monoid_instance::value,
                      "reduction requires that value type is monoid instance");
        return parser<It, V, R>
        {
            .description = "[(reduced) " + p.description + "]",
            .parse = [=](typename parser<It, V, R>::accumulator_type & acc)
            {
                auto start (torange (acc));
                auto res (p.parse (acc));
                if (parse_success (res)) {
                    auto diff (start.distance (torange (res)));
                    auto past (res.past (diff));
                    auto preresult
                        (fnk::map
                            (result_value<V>,
                            fnk::filter
                                (fnk::map
                                    (toresult, past),
                                is_value<V>)));
                    res.insert (fnk::fold (preresult), torange (res));
                    return res;
                } else {
                    return res;
                }
            }
        };
    }
 
    //
    // Reduction parser: reduces over a list of parse values (using a foldr)
    // to produce a final parse result.
    //
    template <typename F, typename B, typename It, typename V, typename R,
        typename = std::enable_if_t<fnk::monoid<V>::is_monoid_instance::value>>
    inline decltype(auto) reducer (parser<It, V, R> const& p, F && f, B && b)
    {
        using W = typename fnk::type_support::function_traits<F>::return_type;
        return parser<It, W, R>
        {
            .description =
                "[(reducer'd by" +
                fnk::utility::format_function_type<F>() + 
                ") " + 
                p.description + 
                "]",
            .parse = [=](typename parser<It, W, R>::accumulator_type & acc)
            {
                auto start (torange (acc));
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, start };
 
                auto res (p.parse (mock));
                if (parse_success (res)) {
                    auto diff (start.distance (torange (res)));
                    auto past (res.past (diff));
                    auto preresult
                        (fnk::map
                            (result_value<V>,
                            fnk::filter
                                (fnk::map
                                    (toresult, past),
                                is_value<V>)));
                    acc.insert (fnk::foldr (f, b, preresult), torange (res));
                    return acc;
                } else {
                    acc.insert
                        (failure {toresult_failure (mock)}, torange (mock));
                    return acc;
                }
            }
        };
    }
    
    //
    // Reduction parser: reduces over a list of parse values (using a foldl)
    // to produce a final parse result.
    //
    template <typename F, typename B, typename It, typename V, typename R,
        typename = std::enable_if_t<fnk::monoid<V>::is_monoid_instance::value>>
    inline decltype(auto) reducel (parser<It, V, R> const& p, F && f, B && b)
    {
        using W = typename fnk::type_support::function_traits<F>::return_type;
        return parser<It, W, R>
        {
            .description =
                "[(reducel'd by" +
                fnk::utility::format_function_type<F>() + 
                ") " + 
                p.description + 
                "]",
            .parse = [=](typename parser<It, W, R>::accumulator_type & acc)
            {
                auto start (torange (acc));
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, start };
 
                auto res (p.parse (mock));
                if (parse_success (res)) {
                    auto diff (start.distance (torange (res)));
                    auto past (res.past (diff));
                    auto preresult
                        (fnk::map
                            (result_value<V>,
                            fnk::filter
                                (fnk::map
                                    (toresult, past),
                                is_value<V>)));
                    acc.insert (fnk::foldl (f, b, preresult), torange (res));
                    return acc;
                } else {
                    acc.insert
                        (failure {toresult_failure (mock)}, torange (mock));
                    return acc;
                }
            }
        };
    }

    //
    // Lift a parser to a new value type using an existing type conversion
    // (by static_cast).
    //
    template <typename U, typename It, typename V, typename R>
    inline decltype(auto) lift (parser<It, V, R> const& p)
    {
        return lift (p, [](V const& v) { return static_cast<U>(v); });
    } 

    //
    // Lift a parser to a new value type using an explicitly provided function;
    // this is the same as an fmap.
    //
    template <typename F, typename It, typename V, typename R>
    inline decltype(auto) lift (parser<It, V, R> const& p, F && f)
    {
        using U = typename fnk::type_support::function_traits<F>::return_type;
        return parser<It, U, R>
        {
            .description =
                "[" + 
                p.description + 
                " //fmap// " + 
                fnk::utility::format_function_type<F>() + 
                "]",
            .parse =
            [=](typename rpc::core::parser<It, U, R>::accumulator_type & acc)
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (acc) };
 
                auto res (p.parse (mock));
                if (parse_success (res)) {
                    acc.insert
                        (fnk::eval (f, toresult_value (res)), torange (res));
                    return acc;
                }
                else {
                    acc.insert
                        (failure {toresult_failure (mock)}, torange (mock));
                    return acc;
                }
            }
        };
    }

    //
    // Lift a parser to a new value type using an existing type conversion,
    // then fold over the results to a reduced final parse value.
    //
    template <typename U, typename It, typename V, typename R>
    inline decltype(auto) liftreduce (parser<It, V, R> const& p)
    {
        return reduce (lift<U> (p));
    }

    //
    // Lift a parser to a new value type using an explicitly provided function,
    // then fold over the results to reduce to a final parse value.
    //
    template <typename It, typename V, typename R, typename F>
    inline decltype(auto) liftreduce (parser<It, V, R> const& p, F && f)
    {
        return reduce (lift (p, f));
    }
 
    //
    // Lift a parser to a new value type using an existing type convertion,
    // then foldl over the results using the second provided function to reduce
    // to a final parse value.
    //
    template
    <typename U, typename G, typename B, typename It, typename V, typename R>
    inline decltype(auto) liftreducel
        (parser<It, V, R> const& p, G && g, B && b)
    {
        return reducel (lift<U> (p), g, b);
    }
    
    //
    // Lift a parser to a new value type using an existing type convertion,
    // then foldr over the results using the second provided function to reduce
    // to a final parse value.
    //
    template
    <typename U, typename G, typename B, typename It, typename V, typename R>
    inline decltype(auto) liftreducer
        (parser<It, V, R> const& p, G && g, B && b)
    {
        return reducer (lift<U> (p), g, b);
    }
 
    //
    // Lift a parser to a new value type using an explicitly provided function,
    // then foldl over the results using the second provided function to reduce
    // to a final parse value.
    //
    template
    <typename F, typename G, typename B, typename It, typename V, typename R>
    inline decltype(auto) liftreducel
        (parser<It, V, R> const& p, F && f, G && g, B && b)
    {
        return reducel (lift (p, f), g, b);
    }
    
    //
    // Lift a parser to a new value type using an explicitly provided function,
    // then foldr over the results using the second provided function to reduce
    // to a final parse value.
    //
    template
    <typename F, typename G, typename B, typename It, typename V, typename R>
    inline decltype(auto) liftreducer
        (parser<It, V, R> const& p, F && f, G && g, B && b)
    {
        return reducer (lift (p, f), g, b);
    }

    //
    // Inject a specified value in the place of a successful result value,
    // otherwise fail normally.
    //
    template <typename W, typename It, typename V, typename R>
    inline decltype(auto) inject (parser<It, V, R> const& p, W && w)
    {
        return parser<It, W, R>
        {
            .parse = [=](typename parser<It, W, R>::accumulator_type & acc)
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (acc) };
                
                auto res (p.parse (mock));
                if (parse_success (res)) {
                    acc.insert (w, torange (res));
                    return acc;
                } else {
                    acc.insert
                        (failure {toresult_failure (mock)}, torange (mock));
                    return acc;
                }
            }
        };
    }
/*
namespace detail
{
    template <typename It, typename V, typename F>
    inline decltype(auto) rest (V const& v, parser<It, V> const& p, parser<It, F> const& op)
    {
        return parser<It, V>
        {
            
        };
    }
} // namespace detail

    template <typename It, typename V, typename F>
    inline decltype(auto) chainls (parser<It, V> const& p, parser<It, F> const& op)
    {
        return parser<It, V>
        {
            .parse = [=](typename parser<It, V>::range_type const& r)
            {
            }
        };
    }
*/
} // namespace core
} // namespace rpc

#endif // ifndef COMBINATORS_HPP

