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
#include "token_parsers.hpp"

#include "../funktional/include/compose.hpp"
#include "../funktional/include/type_support/function_traits.hpp"

#include "../gsl/not_null.hpp"

namespace rpc
{
namespace core
{
    template <typename F, typename It, typename V, typename R>
    auto lift (parser<It, V, R> const&, F &&)
    -> parser
        <It, typename fnk::type_support::function_traits<F>::return_type, R>;


    template <typename It, typename V, typename R, typename F>
    inline auto bind (parser<It, V, R> const& p, F && f)
        -> typename parser_traits
            <typename fnk::type_support::function_traits<F>::return_type>::type
    {
        using Ftraits = fnk::type_support::function_traits<F>;
        using Fret = typename Ftraits::return_type;
        
        static_assert (is_parser_instance<Fret>::value,
                      "function being bound must return a parser.");

        using Qtraits = parser_traits<Fret>;
        using Qv = typename Qtraits::value_type;

        static_assert (std::is_same<V, Qv>::value,
                      "value types of bound parsers must agree.");

        using A = typename Qtraits::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        return parser<It, Qv, R>
        {
            .description =
                "[" +
                p.description +
                " //bind// " +
                fnk::utility::format_function_type<F>() +
                "]",
            .parse = [=](AccT const acc) 
            {
                auto res (p.parse (acc));
                if (parse_success (*res)) {
                    auto q (fnk::eval (f, toresult (*res)));
                    return q.parse (res);
                } else {
                    return res;
                }
            }
        };
    }

    //
    // bind even on failure; if failure occurs, the failed parse
    // is popped from the accumulator, otherwise the results are not modified.
    //
    template <typename It, typename V, typename R, typename F>
    inline auto bindf (parser<It, V, R> const& p, F && f)
        -> typename parser_traits
            <typename fnk::type_support::function_traits<F>::return_type>::type
    {
        using Fret =
            typename fnk::type_support::function_traits<F>::return_type;

        static_assert (is_parser_instance<Fret>::value,
                      "function being bound must return a parser.");

        using Qtraits = parser_traits<Fret>;
        using Qv = typename Qtraits::value_type;
       
        static_assert (std::is_same<V, Qv>::value,
                      "value types of bound parsers must agree.");

        using A = typename parser<It, Qv, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        return parser<It, Qv, R>
        {
            .description = 
                "[" + 
                p.description + 
                " //bindf// " + 
                fnk::utility::format_function_type<F>() + 
                "]",
            .parse = [=](AccT const acc) 
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (*acc) };
                auto mockptr (AccT {&mock});
                auto pres    (p.parse (mockptr));
                auto q       (fnk::eval (f, toresult (*pres)));

                if (parse_success (*pres))
                    acc->insert (*pres);

                return q.parse (acc);
            }
        }; 
    }
 
    template <typename It, typename V, typename R>
    inline parser<It, V, R> ignore (parser<It, V, R> const& p)
    {
        using A = typename parser<It, V, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        return parser<It, V, R>
        {
            .description = p.description,
            .parse = [=](AccT const acc) 
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (*acc) };
                auto mockptr (AccT {&mock});
                auto pres    (p.parse (mockptr));

                if (parse_success (*pres))
                    acc->replace (torange (*pres));
                else
                    acc->insert (toresult (*pres), torange (*pres));
                return acc;
            }
        };
    }

    template <typename It, typename V, typename R>
    inline parser<It, V, R> operator- (parser<It, V, R> const& p)
    {
        return ignore (p);
    }

    template <typename U, typename It, typename V, typename R>
    inline parser<It, U, R> liftignore (parser<It, V, R> const& p)
    {
        return ignore
            (override_description
                (inject (p, empty<U>{}),
                p.description));
    }

    template <typename It, typename V, typename R>
    inline parser<It, V, R> branch (parser<It, V, R> const& p,
                                    parser<It, V, R> const& succ,
                                    parser<It, V, R> const& next)
    {
        return override_description
            (bindf
                (p,
                [=](typename parser<It, V, R>::result_type const& r)
                {
                    return r.is_success() ? succ : next;
                }),
            "[(branch) :: " +
            p.description + 
            " => " + 
            succ.description +
            " | " + 
            next.description);
    }
 
    template <typename It, typename V, typename R = range<It>>
    inline parser<It, V, R> sequence (parser<It, V, R> const& p,
                                      parser<It, V, R> const& q)
    {
        using A = typename parser<It, V, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        return parser<It, V, R>
        {
            .description =
                "[" + p.description + " //then// " + q.description + "]",
            .parse = [=](AccT const acc) 
            {
                auto pres (p.parse (acc));
                if (parse_success (*pres))
                    return q.parse (pres);
                else
                    return pres;
            }
        };
    /*    return override_description
            (branch (p, q, basic::failwith<It, V, R> (p.description)),
             "[" + p.description + " //then// " + q.description + "]");*/
    }

    template <typename P,typename ... Qs,
              typename = std::enable_if_t<sizeof...(Qs) >= 2>>
    inline auto sequence (P && p, Qs && ... qs)
        -> typename parser_traits<P>::type
    {
        return sequence (p, sequence (qs...));    
    }

    template <typename It, typename V, typename R = range<It>>
    inline parser<It, V, R> operator& (parser<It, V, R> const& p,
                                        parser<It, V, R> const& q)
    {
        return sequence (p, q);
    }

    //
    // Sequence two parsers, but ignore the result of the left argument.
    // Note that the value types do not have to be the same; in fact, the parser
    // returned has the value type of the right argument.
    //
    template <typename P, typename Q>
    inline auto ignorel (P && p, Q && q) -> typename parser_traits<Q>::type
    {
        using U = typename parser_traits<Q>::value_type;
        return sequence (liftignore<U> (p), q);
    }

    template <typename P, typename Q>
    inline auto operator<< (P && p, Q && q) -> typename parser_traits<Q>::type
    {
        return ignorel (p, q);
    }

    //
    // Sequence two parsers, but ignore the result of the right argument
    // Note that the value types do not have to be the same; in fact, the parser
    // returned has the value type of the left argument.
    //
    template <typename P, typename Q>
    inline auto ignorer (P && p, Q && q) -> typename parser_traits<P>::type
    {
        using U = typename parser_traits<P>::value_type;
        return sequence (p, liftignore<U> (q));
    }

    template <typename P, typename Q>
    inline auto operator>> (P && p, Q && q) -> typename parser_traits<Q>::type
    {
        return ignorer (p, q);
    }

    //
    // Sequence a parser wrapped on the left and right by the second
    // argument, ignoring the result of the separator.
    // Note that the value types do not have to be the same;
    // in fact, the parser returned has the value type of the first argument.
    //
    template <typename P, typename S>
    inline auto wrappedby (P && p, S && s) -> typename parser_traits<P>::type
    {
        using U = typename parser_traits<P>::value_type;
        return sequence (liftignore<U> (s), p, liftignore<U> (s));
    }
 
    template <typename It, typename V, typename R>
    inline parser<It, V, R> option (parser<It, V, R> const& p,
                                    parser<It, V, R> const& q)
    {
        using A = typename parser<It, V, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;
        return parser<It, V, R>
        {
            .description =
                "[" + p.description + " //or// " + q.description + "]",
            .parse = [=](AccT const acc) 
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (*acc) };
                auto mockptr (AccT {&mock});
                auto pres (p.parse (mockptr));
                
                if (parse_success (*pres)) {
                    acc->insert (*pres);
                    return acc;
                } else {
                    return q.parse (acc); 
                }
            }
        };
    }
 
    template <typename P, typename ... Qs,
              typename = std::enable_if_t<sizeof...(Qs) >= 2>>
    inline auto option (P && p, Qs && ... qs) -> typename parser_traits<P>::type
    {
        return option (p, option (qs...));
    }

    template <typename It, typename V, typename R>
    inline parser<It, V, R> operator| (parser<It, V, R> const& p,
                                       parser<It, V, R> const& q)
    {
        return option (p, q);
    }
 
    //
    // Either 0 or 1 successful parses. If the primary parse fails,
    // then pass silently.
    //
    template <typename It, typename V, typename R>
    inline parser<It, V, R> optional (parser<It, V, R> const& p)
    {
        return override_description
            (option (p, core::pass<It, V, R>), "(optional) " + p.description);
    }

    template <typename It, typename V, typename R>
    inline parser<It, V, R> operator~ (parser<It, V, R> const& p)
    {
        return optional (p);
    }
 
    //
    // Either 0 or 1 successful parses. If the primary parse fails,
    // then the default value is returned; hence, this parser ALWAYS
    // succeeds, but may or may not consume input.
    //
    template <typename It, typename V, typename R>
    inline parser<It, V, R> optional (parser<It, V, R> const& p,
                                      V const& default_value)
    {
        auto dflt (core::unit<It, V, R> (default_value));
        return override_description
            (option (p, dflt),
            "(optional) " + p.description + " | " + dflt.description);
    }

    //
    // At least one but at most n successful parses; if n == 0,
    // then there is no upper bound.
    //
    template <typename It, typename V, typename R>
    inline parser<It, V, R> some (parser<It, V, R> const& p,
                                  std::size_t const n = 0)
    {
        using A = typename parser<It, V, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        auto success
        (parser<It, V, R>
        {
            .description = "(iterated) " + p.description,
            .parse = [=] (AccT const acc) -> AccT
            {
                auto itered_
                    (fnk::iterate_while
                        (p.parse,
                         [](AccT const a) { return parse_success (*a); },
                         n == 0 ? 0 : n-1));

                auto asize_ (acc->size ());
                auto res_   (itered_ (acc));
                auto diff_  (res_->size() - asize_);
                if (not parse_success (*res_) && diff_ > 1)
                    res_->ignore_previous ();
                return res_;
            }
        });

        return override_description
            (branch
                (p, success, core::failwith<It, V, R> (p.description)),
            "[(some) " + p.description + "]");
    }

    template <typename It, typename V, typename R>
    inline parser<It, V, R> operator++ (parser<It, V, R> const& p)
    {
        return some (p); 
    }

    template <typename It, typename V, typename R>
    inline parser<It, V, R> operator+ (std::size_t const n,
                                       parser<It, V, R> const& p)
    {
        return some (p, n); 
    }

    //
    // Zero or more successful parses
    //
    template <typename It, typename V, typename R>
    inline parser<It, V, R> many (parser<It, V, R> const& p)
    {
        using A = typename parser<It, V, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        auto success
        (parser<It, V, R>
        {
            .description = "(iterated) " + p.description,
            .parse = [=] (AccT const acc) -> AccT
            {
                auto itered_
                    (fnk::iterate_while
                        (p.parse,
                         [](AccT const a) { return parse_success (*a); }));

                auto res_ (itered_ (acc));
                if (not parse_success (*res_))
                    res_->ignore_previous ();
                return res_;
            }
        });

        return override_description
            (branch
                (p, success, core::pass<It, V, R>),
            "[(many) " + p.description + "]");
    }

    template <typename It, typename V, typename R>
    inline parser<It, V, R> operator+ (parser<It, V, R> const& p)
    {
        return many (p);
    }


    //
    // Reduction parser: reduces over a list of parse values (using a foldl)
    // to produce a final parse result.
    //
    template <typename It, typename V, typename R,
        typename = std::enable_if_t<fnk::monoid<V>::is_monoid_instance::value>>
    inline parser<It, V, R> reduce (parser<It, V, R> const& p)
    {
        static_assert (fnk::monoid<V>::is_monoid_instance::value,
                      "reduction requires that value type is monoid instance");
       
        using A    = typename parser<It, V, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        return parser<It, V, R>
        {
            .description = "[(reduced) " + p.description + "]",
            .parse = [=](AccT const acc) 
            {
                auto asize (acc->size ());
                auto res   (p.parse (acc));

                if (parse_success (*res)) {
                    assert (res->size() > asize); 
                    auto diff (res->size() - asize);
                    auto past (res->past (diff));
                    std::vector<V> vs;
                    vs.reserve (diff);

                    for (auto it (past.first); it != past.second; ++it) {
                        auto r (toresult (*it)); 
                        if (r.is_value ())
                            vs.emplace_back (r.to_value ());
                    }

                    res->insert (fnk::fold (vs), torange (*res));
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
    inline auto reducer (parser<It, V, R> const& p, F && f, B && b)
        -> parser
            <It, typename fnk::type_support::function_traits<F>::return_type, R>
    {
        using W = typename fnk::type_support::function_traits<F>::return_type;
        
        using A = typename parser<It, W, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;
        
        using MockA = typename parser<It, V, R>::accumulator_type;
        using MockAccT = gsl::not_null_ptr<MockA>;
        
        return parser<It, W, R>
        {
            .description =
                "[(reducer'd by" +
                fnk::utility::format_function_type<F>() + 
                ") " + 
                p.description + 
                "]",
            .parse = [=](AccT const acc) 
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (*acc) };
                auto mockptr (MockAccT {&mock});
                auto res (p.parse (mockptr));

                if (parse_success (*res)) {
                    assert (res->size() >= 1); 
                    std::vector<V> vs;
                    vs.reserve (res->size() - 1);

                    for (auto e : *res) {
                        auto r (toresult (e)); 
                        if (r.is_value ())
                            vs.emplace_back (r.to_value ());
                    }

                    acc->insert (fnk::foldr (f, b, vs), torange (*res));
                    return acc;
                } else {
                    acc->insert
                        (failure {toresult_failure (*res)}, torange (*mockptr));
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
    inline auto reducel (parser<It, V, R> const& p, F && f, B && b)
        -> parser
            <It, typename fnk::type_support::function_traits<F>::return_type, R>
    {
        using W = typename fnk::type_support::function_traits<F>::return_type;

        using A = typename parser<It, W, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;
        
        using MockA = typename parser<It, V, R>::accumulator_type;
        using MockAccT = gsl::not_null_ptr<MockA>;
        
        return parser<It, W, R>
        {
            .description =
                "[(reducel'd by" +
                fnk::utility::format_function_type<F>() + 
                ") " + 
                p.description + 
                "]",
            .parse = [=](AccT const acc) 
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (*acc) };
                auto mockptr (MockAccT {&mock});
                auto res (p.parse (mockptr));

                if (parse_success (*res)) {
                    assert (res->size() >= 1);
                    std::vector<V> vs;
                    vs.reserve (res->size() - 1);

                    for (auto e : *res) {
                        auto r (toresult (e)); 
                        if (r.is_value ())
                            vs.emplace_back (r.to_value ());
                    }

                    acc->insert (fnk::foldl (f, b, vs), torange (*res));
                    return acc;
                } else {
                    acc->insert
                        (failure {toresult_failure (*res)}, torange (*mockptr));
                    return acc;
                }
            }
        };
    }


    template <typename F, typename B, typename It, typename V, typename R>
    inline auto operator<<= (parser<It, V, R> const& p, std::pair<F, B> const& rfb)
    -> parser
        <It, typename fnk::type_support::function_traits<F>::return_type, R>
    {
        return reducel (p, rfb.first, rfb.second); 
    }


    template <typename F, typename B, typename It, typename V, typename R>
    inline auto operator>>= (parser<It, V, R> const& p, std::pair<F, B> const & rfb)   
    -> parser
        <It, typename fnk::type_support::function_traits<F>::return_type, R>
    {
        return reducer (p, rfb.first, rfb.second); 
    }


    //
    // Lift a parser to a new value type using an existing type conversion
    // (by static_cast).
    //
    template <typename U, typename It, typename V, typename R>
    inline parser<It, U, R> lift (parser<It, V, R> const& p)
    {
        return lift (p, [](V const& v) { return static_cast<U>(v); });
    }


    //
    // Lift a parser to a new value type using an explicitly provided function;
    // this is the same as an fmap.
    //
    template <typename F, typename It, typename V, typename R>
    inline auto lift (parser<It, V, R> const& p, F && f)
        -> parser
            <It, typename fnk::type_support::function_traits<F>::return_type, R>
    {
        using U = typename fnk::type_support::function_traits<F>::return_type;

        using A = typename parser<It, U, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        using MockA = typename parser<It, V, R>::accumulator_type;
        using MockAccT = gsl::not_null_ptr<MockA>;

        return parser<It, U, R>
        {
            .description =
                "[" +
                p.description +
                " //fmap// " +
                fnk::utility::format_function_type<F>() +
                "]",
            .parse =
            [=](AccT const acc) 
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (*acc) };
                auto mockptr (MockAccT {&mock}); 
                auto res     (p.parse (mockptr));

                if (parse_success (*res)) {
                    for (auto e : *res) {
                        auto r (toresult (e));
                        if (r.is_value ())
                            acc->insert
                                (fnk::eval (f, r.to_value ()), torange (e));
                        else if (r.is_empty ())
                            acc->insert (empty<U>{}, torange (e));
                    }
                    return acc;
                } else {
                    acc->insert
                        (failure {toresult_failure (*res)}, torange (*mockptr));
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
    inline auto liftreduce (parser<It, V, R> const& p)
        -> decltype(reduce (lift<U> (p)))
    {
        return reduce (lift<U> (p));
    }


    //
    // Lift a parser to a new value type using an explicitly provided function,
    // then fold over the results to reduce to a final parse value.
    //
    template <typename It, typename V, typename R, typename F>
    inline auto liftreduce (parser<It, V, R> const& p, F && f)
        -> decltype(reduce (lift (p, f))) 
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
    inline auto liftreducel (parser<It, V, R> const& p, G && g, B && b)
        -> decltype(reducel (lift<U> (p), g, b))
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
    inline auto liftreducer (parser<It, V, R> const& p, G && g, B && b)
        -> decltype(reducer (lift<U> (p), g, b))
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
    inline auto liftreducel (parser<It, V, R> const& p, F && f, G && g, B && b)
        -> decltype(reducel (lift (p, f), g, b))
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
    inline auto liftreducer (parser<It, V, R> const& p, F && f, G && g, B && b)
        -> decltype(reducer (lift (p, f), g, b))
    {
        return reducer (lift (p, f), g, b);
    }


    //
    // Inject a specified value in the place of a successful result value,
    // otherwise fail normally.
    //
    template <typename U, typename It, typename V, typename R>
    inline auto inject (parser<It, V, R> const& p, U && u)
        -> parser
            <It,
             std::remove_cv_t
                <std::conditional_t
                    <is_empty_instance<U>::value,
                     typename empty_traits<U>::type, U>>, R>
    {
        using W = std::remove_cv_t
            <std::conditional_t
                <is_empty_instance<U>::value,
                 typename empty_traits<U>::type, U>>;

        using C = std::remove_cv_t
            <std::conditional_t
                <is_empty_instance<U>::value,
                 empty<typename empty_traits<U>::type>, U>>;

        using A = typename parser<It, W, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        using MockA = typename parser<It, V, R>::accumulator_type;
        using MockAccT = gsl::not_null_ptr<MockA>;

        return parser<It, W, R>
        {
            .description =
                "[(injected value: " +
                fnk::utility::type_name<U>::name() +
                ") " +
                p.description,
            .parse = [=](AccT const acc) 
            {
                typename parser<It, V, R>::accumulator_type mock
                    { empty<V>{}, torange (*acc) };
                auto mockptr (MockAccT {&mock});
                auto res     (p.parse (mockptr));

                if (parse_success (*res)) {
                    for (auto e : *res)
                        acc->insert (static_cast<C>(u), torange (e));
                    return acc; 
                } else {
                    acc->insert
                        (failure {toresult_failure (*res)}, torange (*mockptr));
                    return acc;
                }
            }
        };
    }


    template <typename U, typename It, typename V, typename R,
        typename = std::enable_if_t
            <not (std::is_function<std::decay_t<U>>::value ||
                  fnk::utility::is_specialization<U, std::function>::value)>>
    inline auto operator% (parser<It, V, R> const& p, U && u)   
         -> parser
            <It,
             std::remove_cv_t
                <std::conditional_t
                    <is_empty_instance<U>::value,
                     typename empty_traits<U>::type, U>>, R>
    {
        return inject (p, std::forward<U> (u)); 
    }


    template <typename F, typename It, typename V, typename R>
    inline auto operator^ (parser<It, V, R> const& p, F && f)   
    -> parser
        <It, typename fnk::type_support::function_traits<F>::return_type, R>
    {
        return lift (p, f); 
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

