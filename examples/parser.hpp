//
// A tiny parser library using `funktional`
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef PARSER_HPP
#define PARSER_HPP

#include <type_traits>
#include <functional>
#include <list>

#include "concat.hpp"
#include "mappable.hpp"
#include "alternative.hpp"
#include "functor.hpp"
#include "applicative_functor.hpp"
#include "monad.hpp"
#include "additive_monad.hpp"
#include "monoid.hpp"

#include "type_support/function_traits.hpp"

namespace fnk
{
    template <typename A>
    struct parser
    {
        std::function<std::list<std::pair<A,std::string>>(std::string const&)> parse;

        using value_type = A;
        struct is_parser_instance : public std::true_type {};
    };

    template <typename A, typename F,
        typename = std::enable_if_t<fnk::type_support::function_traits<F>::return_type::is_parser_instance::value>>
    inline decltype(auto) bind (parser<A> const& p, F && f)
    {
        using R = typename fnk::type_support::function_traits<F>::return_type;
        return parser<R>
        {
            parse ([=](std::string const& s)
            {
                return fnk::concat
                    (fnk::map
                        ([=](std::pair<A,std::string> && r)
                         {
                            return fnk::eval (fnk::eval(f, r.first).parse, r.second);
                         },
                        fnk::eval (p.parse, s)));
            })
        }; 
    }

    template <typename A>
    inline decltype(auto) unit (A && a)
    {
        return parser<A>
        {
            parse ([=](std::string const& s) { return std::list<std::pair<A,std::string>> (std::make_pair(a,s)); })
        };
    }
    
    template <typename P,
        typename = std::enable_if_t<P::is_parser_instance::value>>
    inline decltype(auto) combine (P && p, P && q)
    {
        using A = typename P::value_type;
        return parser<A>
        {
            parse ([=](std::string const& s)
            {
                return fnk::append (fnk::eval (p.parse, s), fnk::eval (q.parse, s));
            })
        };
    }

    template <typename P,
        typename = std::enable_if_t<P::is_parse_instance::value>>
    inline decltype(auto) option (P && p, P && q)
    {
        using A = typename P::value_type;
        return parser<A>
        {
            parse ([=](std::string const& s)
            {
                auto r = fnk::eval (p.parse, s);
                return r.empty() ? fnk::eval (q.parse, s) : r;
            })
        };
    }

    template <typename A>
    struct failure : public parser<A>
    {
        std::function<std::list<std::pair<A,std::string>>(std::string const&)> parse =
            [=](std::string const& s) { return std::list<std::pair<A,std::string>>{}; };
    };

    template <typename A>
    struct fnk::functor<parser<A>> : public fnk::default_functor<parser<A>>
    {        
        template <class F, typename U = typename fnk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t<fnk::functor<U>::is_functor_instance::value>>
        static constexpr U fmap (F && f, parser<A> && p)
        {
            return parser<U>
            {
                parse ([=](std::string const& s)
                {
                    std::list<U, std::string> out;
                    for (auto& e : fnk::eval (p.parse, s))
                    {
                        fnk::type_support::container_traits<decltype(out)>::insert
                            (out, std::make_pair (fnk::eval (f, e.first), e.second));
                    }
                    return out;
                })
            }; 
        }
    };

    template <typename A>
    struct fnk::applicative_functor<parser<A>> : public fnk::default_applicative_functor<parser<A>>
    {
        static inline constexpr decltype(auto) pure (A && a)
        {
            return unit (std::forward<A>(a));
        }

        template <class F, typename U,
            typename = std::enable_if_t<F::is_parser_instance::value>,
            typename = std::enable_if_t<U::is_parser_instance::value>>
        static inline constexpr decltype(auto) apply (F const& fs, U const& us)
        {
            using B = typename F::value_type;
            return parser<B>
            {
                parse ([=](std::string const& s)
                {
                    std::list<B,std::string> out;
                    for (auto& e1 : fnk::eval (fs.parse, s))
                    {
                        for (auto& e2 : fnk::eval (us.parse, e1.second))
                        {
                            fnk::type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair(fnk::eval (e1.first, e2.first), e2.second));
                        }
                    }
                    return out;
                })
            };
        }
    };

    template <typename A>
    struct fnk::monad<parser<A>> : public fnk::default_monad<parser<A>>
    {
        template <class F, typename P,
            typename = std::enable_if_t<std::is_same<std::decay_t<P>, parser<A>>::value>>
        static inline constexpr decltype(auto) mbind (P && p, F && f)
        {
            return bind (std::forward<P>(p), std::forward<F>(f));
        }
    };

    template <typename A>
    struct fnk::monoid<parser<A>> 
    {
        static inline constexpr decltype(auto) unity (void) { return failure<A>{}; } 
   
        template <typename P,
            typename = std::enable_if_t<std::is_same<std::decay_t<P>, parser<A>>::value>>
        static inline constexpr decltype(auto) append (P && p, P && q)
        {
            return option (std::forward<P>(p), std::forward<P>(q));
        }

        struct is_monoid_instance : public std::true_type {};
    };

    template <typename A>
    struct fnk::alternative<parser<A>> : public fnk::default_alternative<parser<A>> {};

    template <typename A>
    struct fnk::additive_monad<parser<A>> : public fnk::default_additive_monad<parser<A>> {};
} // namespace fnk

#endif // ifndef PARSER_HPP

