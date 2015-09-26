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
    template <typename V, typename T = V>
    struct parser
    {
        using value_type = V;
        using token_type = T;
        std::function<std::list<std::pair<V,std::list<T>>>(std::list<T> const&)> parse;
    };

    template <typename T>
    struct is_parser_instance : public std::false_type {};

    template <typename V, typename T>
    struct is_parser_instance<parser<V,T>> : public std::true_type {};

    template <typename V, typename T>
    struct is_parser_instance<parser<V,T>&> : public std::true_type {};

    template <typename V, typename T>
    struct is_parser_instance<parser<V,T> const> : public std::true_type {};

    template <typename V, typename T>
    struct is_parser_instance<parser<V,T> const&> : public std::true_type {};

    template <typename V, typename T>
    struct is_parser_instance<parser<V,T> &&> : public std::true_type {};

    template <typename V, typename T, typename F,
        typename = std::enable_if_t<is_parser_instance<typename fnk::type_support::function_traits<F>::return_type>::value>>
    inline decltype(auto) bind (parser<V,T> const& p, F && f)
    {
        using R = typename fnk::type_support::function_traits<F>::return_type;
        return parser<typename R::value_type, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                return fnk::concat
                    (fnk::map
                        ([=](std::pair<V,std::list<T>> & r)
                         {
                            return fnk::eval (fnk::eval(f, r.first).parse, r.second);
                         },
                        fnk::eval (p.parse, s)));
            }
        }; 
    }

    template <typename V, typename T = V>
    inline decltype(auto) unit (V && v)
    {
        return parser<V,T>
        {
            .parse = [=](std::list<T> const& s)
            {
                return std::list<std::pair<V,std::list<T>>> { std::make_pair(v,s) };
            }
        };
    }
 
    template <typename V, typename T>
    static auto failure = parser<V, T>
    {
        .parse = [](std::list<T> const& /*s*/) { return std::list<std::pair<V,std::list<T>>>{}; }
    };
   
    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename = std::enable_if_t<std::is_same<typename std::decay_t<P>::value_type, typename std::decay_t<Q>::value_type>::value>,
        typename = std::enable_if_t<std::is_same<typename std::decay_t<P>::token_type, typename std::decay_t<Q>::token_type>::value>>
    inline decltype(auto) combine (P && p, Q && q)
    {
        using V = typename std::decay_t<P>::value_type;
        using T = typename std::decay_t<P>::token_type;
        return parser<V, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                return fnk::append (fnk::eval (p.parse, s), fnk::eval (q.parse, s));
            }
        };
    }

    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename = std::enable_if_t<std::is_same<typename std::decay_t<P>::value_type, typename std::decay_t<Q>::value_type>::value>,
        typename = std::enable_if_t<std::is_same<typename std::decay_t<P>::token_type, typename std::decay_t<Q>::token_type>::value>>
    inline decltype(auto) sequence (P && p, Q && q)
    {
        using V = typename std::decay_t<P>::value_type;
        using T = typename std::decay_t<P>::token_type;
        return parser<V, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                auto r1 = fnk::eval (p.parse, s);
                if (r1.empty())
                    return r1;
                else {
                    auto r2 = fnk::eval (q.parse, r1.back().second);
                    if (r2.empty())
                       return r2;
                    else {
                        r1.splice (r1.cend(), r2); 
                        return r1; 
                    }
                }
            }
        };
    }
    
    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename = std::enable_if_t<std::is_same<typename std::decay_t<P>::value_type, typename std::decay_t<Q>::value_type>::value>,
        typename = std::enable_if_t<std::is_same<typename std::decay_t<P>::token_type, typename std::decay_t<Q>::token_type>::value>>
    inline decltype(auto) option (P && p, Q && q)
    {
        using V = typename std::decay_t<P>::value_type;
        using T = typename std::decay_t<P>::token_type;
        return parser<V, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                auto r = fnk::eval (p.parse, s);
                return r.empty() ? fnk::eval (q.parse, s) : r;
            }
        };
    }

    template <typename V, typename T>
    struct fnk::functor<parser<V, T>> : public fnk::default_functor<parser, V, T>
    { 
        template <class F, typename U = typename fnk::type_support::function_traits<F>::return_type,
            typename = std::enable_if_t
                <std::is_convertible
                    <V, typename fnk::type_support::function_traits<F>::template argument<0>::type>::value>>
        static constexpr parser<U, T> fmap (F && f, parser<V, T> const& p)
        {
            return parser<U, T>
            {
                .parse = [=](std::list<T> const& s)
                {
                    std::list<std::pair<U, std::list<T>>> out;
                    for (auto& e : fnk::eval (p.parse, s))
                    {
                        fnk::type_support::container_traits<decltype(out)>::insert
                            (out, std::make_pair (fnk::eval (f, e.first), e.second));
                    }
                    return out;
                }
            }; 
        }
    };

    template <typename V, typename T>
    struct fnk::functor<parser<V, T> const> : public fnk::functor<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::functor<parser<V, T> &> : public fnk::functor<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::functor<parser<V, T> const&> : public fnk::functor<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::functor<parser<V, T> &&> : public fnk::functor<parser<V, T>> {};
    
    template <typename V, typename T>
    struct fnk::applicative_functor<parser<V, T>> : public fnk::default_applicative_functor<parser<V, T>>
    {
        static inline constexpr decltype(auto) pure (V && v)
        {
            return unit (std::forward<V>(v));
        }

        template <class F, typename U,
            typename = std::enable_if_t<is_parser_instance<F>::value>,
            typename = std::enable_if_t<is_parser_instance<U>::value>>
        static inline constexpr decltype(auto) apply (F const& fs, U const& us)
        {
            using W = typename std::decay_t<F>::value_type;
            using S = typename std::decay_t<F>::token_type; 
            return parser<W, S>
            {
                .parse = [=](std::list<S> const& s)
                {
                    std::list<std::pair<W,std::list<S>>> out;
                    for (auto const& e1 : fnk::eval (fs.parse, s))
                    {
                        for (auto const& e2 : fnk::eval (us.parse, e1.second))
                        {
                            fnk::type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair(fnk::eval (e1.first, e2.first), e2.second));
                        }
                    }
                    return out;
                }
            };
        }
    };

    template <typename V, typename T>
    struct fnk::applicative_functor<parser<V, T> const> : public fnk::applicative_functor<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::applicative_functor<parser<V, T> &> : public fnk::applicative_functor<parser<V, T>> {};   
    template <typename V, typename T>
    struct fnk::applicative_functor<parser<V, T> const&> : public fnk::applicative_functor<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::applicative_functor<parser<V, T> &&> : public fnk::applicative_functor<parser<V, T>> {};
    
    template <typename V, typename T>
    struct fnk::alternative<parser<V, T>> : public fnk::default_alternative<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::alternative<parser<V, T> const> : public fnk::alternative<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::alternative<parser<V, T> &> : public fnk::alternative<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::alternative<parser<V, T> const&> : public fnk::alternative<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::alternative<parser<V, T> &&> : public fnk::alternative<parser<V, T>> {};
    
    template <typename V, typename T>
    struct fnk::monad<parser<V, T>> : public fnk::default_monad<parser<V, T>>
    {
        template <class F, typename P,
            typename = std::enable_if_t<std::is_same<std::decay_t<P>, parser<V, T>>::value>>
        static inline constexpr decltype(auto) mbind (P && p, F && f)
        {
            return bind (std::forward<P>(p), std::forward<F>(f));
        }
    };

    template <typename V, typename T>
    struct fnk::monad<parser<V, T> const> : public fnk::monad<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::monad<parser<V, T> &> : public fnk::monad<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::monad<parser<V, T> const&> : public fnk::monad<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::monad<parser<V, T> &&> : public fnk::monad<parser<V, T>> {};
    
    template <typename V, typename T>
    struct fnk::monoid<parser<V, T>> 
    {
        static inline constexpr decltype(auto) unity (void) { return failure<V, T>; } 
   
        template <typename P, typename Q,
            typename = std::enable_if_t<is_parser_instance<P>::value>,
            typename = std::enable_if_t<is_parser_instance<Q>::value>,
            typename = std::enable_if_t
                <std::is_same<typename std::decay_t<P>::value_type, typename std::decay_t<Q>::value_type>::value>,
            typename = std::enable_if_t
                <std::is_same<typename std::decay_t<P>::token_type, typename std::decay_t<Q>::token_type>::value>>
        static inline constexpr decltype(auto) append (P && p, Q && q)
        {
            return option (std::forward<P>(p), std::forward<Q>(q));
        }

        struct is_monoid_instance : public std::true_type {};
    };

    template <typename V, typename T>
    struct fnk::monoid<parser<V, T> const> : public fnk::monoid<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::monoid<parser<V, T> &> : public fnk::monoid<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::monoid<parser<V, T> const&> : public fnk::monoid<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::monoid<parser<V, T> &&> : public fnk::monoid<parser<V, T>> {};
    
    template <typename V, typename T>
    struct fnk::additive_monad<parser<V, T> const> : public fnk::additive_monad<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::additive_monad<parser<V, T> &> : public fnk::additive_monad<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::additive_monad<parser<V, T> const&> : public fnk::additive_monad<parser<V, T>> {};
    template <typename V, typename T>
    struct fnk::additive_monad<parser<V, T> &&> : public fnk::additive_monad<parser<V, T>> {};
    
    template <typename T>
    auto item = parser<T, T>
    {
        .parse = [](std::list<T> const& s)
        {
            return s.empty() ? std::list<std::pair<T,std::list<T>>>{}
                             : std::list<std::pair<T,std::list<T>>>
                                    { std::make_pair(s.front(), std::list<T>(++s.begin(), s.end())) }; 
        }
    };

    template <typename V, typename T>
    inline decltype(auto) some (parser<V, T> const& p)
    {
        return parser<V, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                auto r1 = fnk::eval (p.parse, s);
                if (r1.empty())
                   return r1;
                else
                {
                    while (true)
                    {
                        auto r2 = fnk::eval (p.parse, r1.back().second);
                        if (r2.empty())
                            return r1;
                        else
                            r1.splice (r1.cend(), r2);
                    }
                } 
            }
        }; 
    }

    template <typename V, typename T>
    inline decltype(auto) many (parser<V, T> const& p)
    {
        return fnk::alternative<parser<V, T>>::alt
            (some (p), fnk::applicative_functor<parser<V, T>>::pure (std::list<std::pair<V,std::list<T>>>{}));
    }

    template <typename P,
        typename = std::enable_if_t<std::is_same<typename fnk::type_support::function_traits<P>::return_type, bool>::value>>
    inline constexpr decltype(auto) satisfy (P && p)
    {
        using T = std::decay_t<typename fnk::type_support::function_traits<P>::template argument<0>::type>;
        return bind
            (item<T>,
             [=](T t)
             {
                return fnk::eval(p, t) ? unit (std::move(t)) : failure<T,T>; 
             });
    }
} // namespace fnk

#endif // ifndef PARSER_HPP

