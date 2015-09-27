//
// A tiny parser library using `funktional`
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <type_traits>
#include <functional>
#include <list>

#include "algebraic.hpp"
#include "concat.hpp"
#include "filterable.hpp"
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
namespace detail
{
    struct failure
    {
        std::string msg;
    };

    enum
    {
        PARSE_VALUE = 0,
        PARSE_FAILURE
    };
} // namespace detail
    template <typename V, typename T = V>
    struct parser
    {
        using value_type = V;
        using token_type = T;
        using return_type = fnk::adt<V, detail::failure>;

        template <typename U, typename S>
        using rebind = parser<U, S>;

        std::function <std::list<std::pair<return_type, std::list<T>>>(std::list<T> const&)> parse;
       
        static inline decltype(auto) is_value_from_pair (std::pair<return_type, std::list<T>> const& p)
        {
            return p.first.tindex() == detail::PARSE_VALUE;
        }
        
        static inline decltype(auto) is_value (return_type const& r)
        {
            return r.tindex() == detail::PARSE_VALUE;
        }

        static inline decltype(auto) value_from_pair (std::pair<return_type, std::list<T>> const& p)
        {
            return p.first.template value<V>();
        }
        
        static inline decltype(auto) value (return_type const& r)
        {
            return r.template value<V>();
        }
        
        static inline decltype(auto) failure_message_from_pair (std::pair<return_type, std::list<T>> const& p)
        {
            return p.first.template value<detail::failure>();
        }

        static inline decltype(auto) failure_message (return_type const& r)
        {
            return r.template value<detail::failure>();
        }
        
        static inline decltype(auto) values (std::list<std::pair<return_type, std::list<T>>> const& l)
        {
            std::list<V> out;
            for (auto const& e : l)
                if (is_value(e.first))
                    fnk::type_support::container_traits<decltype(out)>::insert (out, value (e.first));
            return out;
        }
 
        static inline decltype(auto) failure_messages (std::list<std::pair<return_type, std::list<T>>> const& l)
        {
            std::list<detail::failure> out;
            for (auto const& e : l)
                if (not is_value(e.first))
                    fnk::type_support::container_traits<decltype(out)>::insert (out, failure_message (e.first));
            return out;
        }
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

    template <typename PairT,
        typename = std::enable_if_t<fnk::utility::is_specialization<PairT, std::pair>::value>,
        typename = std::enable_if_t<fnk::is_algebraic<typename std::tuple_element<0, std::decay_t<PairT>>::type>::value>> 
    inline decltype(auto) is_value (PairT && p)
    {
        return std::forward<PairT>(p).first.tindex() == detail::PARSE_VALUE; 
    }

    template <typename PairT,
        typename = std::enable_if_t<fnk::utility::is_specialization<PairT, std::pair>::value>,
        typename = std::enable_if_t<fnk::is_algebraic<typename std::tuple_element<0, std::decay_t<PairT>>::type>::value>> 
    inline decltype(auto) value (PairT && p)
    {
        using A = typename std::tuple_element<0, std::decay_t<PairT>>::type;
        return std::forward<PairT>(p).first.template value<typename A::template type<detail::PARSE_VALUE>>();
    }
    
    template <typename PairT,
        typename = std::enable_if_t<fnk::utility::is_specialization<PairT, std::pair>::value>,
        typename = std::enable_if_t<fnk::is_algebraic<typename std::tuple_element<0, std::decay_t<PairT>>::type>::value>> 
    inline decltype(auto) failure_message (PairT && p)
    {
        using A = typename std::tuple_element<0, std::decay_t<PairT>>::type;
        return std::forward<PairT>(p).first.template value<typename A::template type<detail::PARSE_FAILURE>>();
    }

    template <typename ParserT,
        typename = std::enable_if_t<is_parser_instance<ParserT>::value>>
    inline decltype(auto) values
        (std::list<std::pair<typename std::decay_t<ParserT>::return_type,
                             std::list<typename std::decay_t<ParserT>::token_type>>> const& l)
    {
        return std::decay_t<ParserT>::values (l);
    }
    
    template <typename ParserT,
        typename = std::enable_if_t<is_parser_instance<ParserT>::value>>
    inline decltype(auto) failure_messages
        (std::list<std::pair<typename std::decay_t<ParserT>::return_type,
                             std::list<typename std::decay_t<ParserT>::token_type>>> const& l)
    {
        return std::decay_t<ParserT>::failure_messages (l);
    }

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
                        ([=](std::pair<typename R::return_type, std::list<T>> & r)
                         {
                            if (parser<V,T>::is_value(r.first))
                                return fnk::eval (fnk::eval(f, parser<V,T>::value(r.first)).parse, r.second);
                            else
                                return std::list<std::pair<typename R::return_type, std::list<T>>>
                                    { std::make_pair (detail::failure { .msg = std::string("failed") }, r.second) };
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
                return std::list<std::pair<typename parser<V,T>::return_type,std::list<T>>> { std::make_pair(v,s) };
            }
        };
    }
 
    template <typename V, typename T>
    static auto fail = parser<V, T>
    {
        .parse = [](std::list<T> const& s)
        {
            return std::list<std::pair<typename parser<V,T>::return_type, std::list<T>>>
                { std::make_pair (detail::failure{.msg = std::string("failed")}, s) };
        }
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
                if (not parser<V,T>::is_value(r1.front().first))
                    return r1;
                else {
                    auto r2 = fnk::eval (q.parse, r1.back().second);
                    if (not parser<V,T>::is_value(r2.front().first))
                       return r2;
                    else {
                        r1.splice (r1.cend(), r2); 
                        return r1; 
                    }
                }
            }
        };
    }

    //
    // Sequence two parsers, but ignore the result of the left argument.
    // Note that the value types do not have to be the same; in fact, the parser
    // returned has the value type of the right argument.
    //
    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename = std::enable_if_t<std::is_same<typename std::decay_t<P>::token_type, typename std::decay_t<Q>::token_type>::value>>
    inline decltype(auto) ignorel (P && p, Q && q)
    {
        using V = typename std::decay_t<Q>::value_type;
        using T = typename std::decay_t<P>::token_type;
        using R = typename std::decay_t<Q>::return_type;
        return parser<V, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                auto r1 = fnk::eval (p.parse, s);
                if (not parser<typename std::decay_t<P>::value_type,T>::is_value(r1.front().first))
                    return std::list<std::pair<R, std::list<T>>>
                        { std::make_pair (detail::failure{.msg = std::string("failure")}, r1.front().second) };
                else 
                    return fnk::eval (q.parse, r1.back().second);
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
        typename = std::enable_if_t<std::is_same<typename std::decay_t<P>::token_type, typename std::decay_t<Q>::token_type>::value>>
    inline decltype(auto) ignorer (P && p, Q && q)
    {
        using V = typename std::decay_t<P>::value_type;
        using T = typename std::decay_t<P>::token_type;
        using R = typename std::decay_t<P>::return_type;
        return parser<V, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                auto r1 = fnk::eval (p.parse, s);
                if (not parser<V,T>::is_value(r1.front().first))
                    return r1;
                else {
                    auto r2 = fnk::eval (q.parse, r1.back().second);
                    if (not parser<typename std::decay_t<Q>::value_type,T>::is_value(r2.front().first))
                        return std::list<std::pair<R, std::list<T>>>
                            { std::make_pair(detail::failure{.msg = std::string("failed")}, r2.front().second) };
                    auto back = r1.pop_back();
                    r1.emplace_back (back.first, r2.second);
                    return r1;
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
                return r.front().first.tindex() == detail::PARSE_FAILURE ? fnk::eval (q.parse, s) : r;
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
                    std::list<std::pair<typename parser<V,T>::template rebind<U,T>::return_type, std::list<T>>> out;
                    for (auto& e : fnk::eval (p.parse, s))
                    {
                        if (parser<V,T>::is_value(e.first))
                            fnk::type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair (fnk::eval (f, parser<V,T>::value(e.first)), e.second));
                        else
                            fnk::type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair (detail::failure{.msg = std::string("failure")}, e.second));
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
                        if (F::is_value (e1)) {
                            for (auto const& e2 : fnk::eval (us.parse, e1.second))
                            {
                                if (U::is_value (e2))
                                    fnk::type_support::container_traits<decltype(out)>::insert
                                        (out, std::make_pair(fnk::eval (e1.first, e2.first), e2.second));
                                else
                                    fnk::type_support::container_traits<decltype(out)>::insert
                                        (out, std::make_pair(detail::failure{.msg = std::string("failure")}, e2.second));
                            }
                        } else 
                            fnk::type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair(detail::failure{.msg = std::string("failure")}, e1.second));                
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
        static inline constexpr decltype(auto) unity (void) { return fail<V, T>; } 
   
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
            return s.empty() ? std::list<std::pair<typename parser<T,T>::return_type,std::list<T>>>
                                    { std::make_pair(detail::failure{.msg = std::string("expected item")}, s) }
                             : std::list<std::pair<typename parser<T,T>::return_type,std::list<T>>>
                                    { std::make_pair(s.front(), std::list<T>(++s.begin(), s.end())) }; 
        }
    };

    //
    // One or more successful parses
    //
    template <typename V, typename T>
    inline decltype(auto) some (parser<V, T> const& p)
    {
        return parser<V, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                auto r1 = fnk::eval (p.parse, s);
                if (not parser<V,T>::is_value(r1.front().first))
                   return r1;
                else {
                    while (true)
                    {
                        auto r2 = fnk::eval (p.parse, r1.back().second);
                        if (not parser<V, T>::is_value(r2.front().first))
                            return r1;
                        else
                            r1.splice (r1.cend(), r2);
                    }
                } 
            }
        }; 
    }

    //
    // Zero or more successful parses
    //
    template <typename V, typename T>
    inline decltype(auto) many (parser<V, T> const& p)
    {
        return parser<V, T>
        {
            .parse = [=](std::list<T> const& s)
            {
                auto r1 = fnk::eval (p.parse, s);
                if (not parser<V,T>::is_value(r1.front().first))
                    return r1;
                else {
                    while (true)
                    {
                        auto r2 = fnk::eval (p.parse, r1.back().second);
                        if (not parser<V,T>::is_value(r2.front().first))
                            return r1;
                        else
                            r1.splice (r1.cend(), r2);
                    }
                }
            }
        }; 
    }

    template <typename P,
        typename = std::enable_if_t<std::is_same<typename fnk::type_support::function_traits<P>::return_type, bool>::value>>
    inline constexpr decltype(auto) satisfy (P && p)
    {
        using T = std::decay_t<typename fnk::type_support::function_traits<P>::template argument<0>::type>;
        using OT = std::list<std::pair<typename parser<T,T>::return_type, std::list<T>>>;
        return parser<T,T>
        {
            .parse = [=](std::list<T> const& s)
            {
                if (s.empty())
                    return OT { std::make_pair(detail::failure{.msg = std::string("no remaining tokens to parse") }, s) };
                else {
                    if (fnk::eval (p, s.front()))
                        return OT { std::make_pair (s.front(), std::list<T>(++s.cbegin(), s.cend())) };
                    else
                        return OT { std::make_pair (detail::failure{.msg = std::string("predicate failed")}, s) };
                }
            } 
        };
    }
} // namespace fnk

#endif // ifndef PARSER_HPP

