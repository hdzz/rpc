//
// A tiny parser library using `funktional`
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef PARSER_HPP
#define PARSER_HPP

#include <iterator>
#include <type_traits>
#include <functional>

#include "algebraic.hpp"
#include "concat.hpp"
#include "mappable.hpp"

#include "alternative.hpp"
#include "functor.hpp"
#include "applicative_functor.hpp"
#include "monad.hpp"
#include "additive_monad.hpp"
#include "monoid.hpp"

#include "type_support/container_traits.hpp"
#include "type_support/function_traits.hpp"

namespace fnk
{
namespace parse
{
namespace detail
{
    struct failure
    {
        failure (void) : msg("failure") {}
        failure (std::string const& s) : msg(s) {}
        failure (std::string && s) : msg(s) {}

        std::string msg;
    };

    template <typename V>
    struct empty_result {};

    enum
    {
        PARSE_RESULT       = 0,
        PARSE_EMPTY_RESULT = 1,
        PARSE_FAILURE      = 2
    };
} // namespace detail

    //
    // A view into the sequence of tokens to be parsed.
    //
    template <typename IterT,
        typename = std::enable_if_t
            <std::is_base_of<std::bidirectional_iterator_tag,
                             typename std::iterator_traits<IterT>::iterator_category>::value>>
    struct range
    {
    public:
        using iter_type = IterT;
        using traits = std::iterator_traits<iter_type>;
        using token_type = typename traits::value_type;
        using diff_type = typename traits::difference_type;
        
        range (void) { static_assert (sizeof(IterT) == 0, "range is not defaul constructable"); }
    
        template <typename C,
            typename = std::enable_if_t<type_support::container_traits<C>::is_container::value>,
            typename = std::enable_if_t<std::is_same<token_type, typename type_support::container_traits<C>::value_type>::value>>
        constexpr range (C const& c) noexcept : valid(not c.empty()), start(c.begin()), end(c.end()) {}

        template <typename It,
            typename = std::enable_if_t<std::is_base_of<iter_type, It>::value>>
        constexpr range (It const& s, It const& e) noexcept : valid(0 < std::distance(s, e)), start(s), end(e) {}

        inline constexpr decltype(auto) head (void) const noexcept(noexcept(*std::declval<const iter_type>())) { return *start; }
        
        inline constexpr decltype(auto) tail (void) const noexcept { return range (std::next(start), end); }
    
        inline constexpr decltype(auto) tail (diff_type const n) { return range (std::next(start, n), end); }

        inline constexpr decltype(auto) length (void) const noexcept { return std::distance (start, end); }
        
        inline constexpr decltype(auto) is_empty (void) const noexcept { return not (valid && start != end); }

        inline constexpr decltype(auto) is_valid (void) const noexcept { return valid; }
    
    private:
        bool valid;
        iter_type const start;
        iter_type const end; 
    };

    template <typename V, typename IterT, typename T = typename std::iterator_traits<IterT>::value_type>
    struct parser
    {
        using range_type = range<IterT>;
        using value_type  = V;
        using token_type  = T;
        using result_type = fnk::adt<std::pair<V, range_type>, std::pair<detail::empty_result<V>, range_type>, detail::failure>;

        template <typename U, typename S>
        using rebind = parser<U, S>;

        using parse_type = std::function<std::list<result_type>(range_type const&)>;
        
        parse_type parse;
       
        static inline decltype(auto) is_result (result_type const& r)
        {
            return r.tindex() == detail::PARSE_RESULT || r.tindex() == detail::PARSE_EMPTY_RESULT;
        }
        
        static inline decltype(auto) is_value_result (result_type const& r)
        {
            return r.tindex() == detail::PARSE_RESULT;
        }
        
        static inline decltype(auto) is_empty_result (result_type const& r)
        {
            return r.tindex() == detail::PARSE_EMPTY_RESULT;
        }

        static inline decltype(auto) is_failure (result_type const& r)
        {
            return r.tindex() == detail::PARSE_FAILURE;
        }

        static inline decltype(auto) result (result_type const& r)
        {
            return r.template value<std::pair<V, range_type>>();
        }
        
        static inline decltype(auto) empty_result (result_type const& r)
        {
            return r.template value<std::pair<detail::empty_result<V>, range_type>>();
        }

        static inline decltype(auto) result_value (result_type const& r)
        {
            return result(r).first;
        }
 
        static inline decltype(auto) result_range (result_type const& r)
        {
            return is_value_result(r) ? result(r).second : empty_result(r).second;
        }
        
        static inline decltype(auto) failure_message (result_type const& r)
        {
           return r.template value<detail::failure>();
        }

        static inline decltype(auto) values (std::list<result_type> const& l)
        {
            std::list<V> out;
            for (auto const& e : l)
                if (is_value_result(e))
                    fnk::type_support::container_traits<decltype(out)>::insert (out, result_value (e));
            return out;
        }
 
        static inline decltype(auto) failure_messages (std::list<result_type> const& l)
        {
            std::list<detail::failure> out;
            for (auto const& e : l)
                if (is_failure(e))
                    fnk::type_support::container_traits<decltype(out)>::insert (out, failure_message (e));
            return out;
        }
    };

    template <typename V, typename It>
    struct parser<V const&, It const> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V const, It const&> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V&, It const> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V const, It&> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V const&, It> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V, It const&> : public parser<V, It> {};
   
    template <typename V, typename It>
    struct parser<V&, It> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V, It&> : public parser<V, It> {}; 
    
    template <typename V, typename It>
    struct parser<V&&, It> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V, It&&> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V&&, It const> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V const, It&&> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V&&, It const&> : public parser<V, It> {};
    
    template <typename V, typename It>
    struct parser<V const&, It&&> : public parser<V, It> {};

    template <typename It>
    struct is_parser_instance : public std::false_type {};

    template <typename V, typename It>
    struct is_parser_instance<parser<V, It>> : public std::true_type {};

    template <typename V, typename It>
    struct is_parser_instance<parser<V, It>&> : public std::true_type {};

    template <typename V, typename It>
    struct is_parser_instance<parser<V, It> const> : public std::true_type {};

    template <typename V, typename It>
    struct is_parser_instance<parser<V, It> const&> : public std::true_type {};

    template <typename V, typename It>
    struct is_parser_instance<parser<V, It> &&> : public std::true_type {};

    template <typename P,
        typename = std::enable_if_t<is_parser_instance<P>::value>>
    struct parser_traits
    {
        using range_type  = typename std::decay_t<P>::range_type;
        using value_type  = typename std::decay_t<P>::value_type;
        using token_type  = typename std::decay_t<P>::token_type;
        using result_type = typename std::decay_t<P>::result_type;

        using parse_type = typename std::decay_t<P>::parse_type;
        
        template <typename U, typename S>
        using rebind = typename std::decay_t<P>::template rebind<U, S>;
    };

    template <typename V, typename It, typename F,
        typename = std::enable_if_t<is_parser_instance<typename fnk::type_support::function_traits<F>::result_type>::value>>
    inline decltype(auto) bind (parser<V, It> const& p, F && f)
    {
        using R = typename fnk::type_support::function_traits<F>::result_type;
        return parser<typename R::value_type, It>
        {
            .parse = [=](typename parser_traits<R>::range_type const& r)
            {
                return fnk::concat
                    (fnk::map
                        ([=](typename parser<V, It>::result_type const& e)
                         {
                            if (parser<V, It>::is_value_result (e))
                                return fnk::eval
                                    (fnk::eval(f, parser<V, It>::result_value (e)).parse, parser<V, It>::result_range (e));
                            else
                                return std::list<typename parser_traits<R>::result_type>{ detail::failure{} };
                         },
                        fnk::eval (p.parse, r)));
            }
        }; 
    }

    template <typename V, typename It = V>
    inline decltype(auto) unit (V && v)
    {
        return parser<V, It>
        {
            .parse = [=](typename parser<V, It>::range_type const& r)
            {
                return std::list<typename parser<V, It>::result_type> { std::make_pair (std::forward<V>(v), r) };
            }
        };
    }
 
    template <typename V, typename It>
    static auto fail = parser<V, It>
    {
        .parse = [](typename parser<V, It>::range_type const& /*r*/)
        {
            return std::list<typename parser<V, It>::result_type> { detail::failure{} };
        }
    };
   
    template <typename P, typename Q,
        typename = std::enable_if_t<is_parser_instance<P>::value>,
        typename = std::enable_if_t<is_parser_instance<Q>::value>,
        typename =
            std::enable_if_t<std::is_same<typename parser_traits<P>::value_type, typename parser_traits<Q>::value_type>::value>,
        typename =
            std::enable_if_t<std::is_same<typename parser_traits<P>::token_type, typename parser_traits<Q>::token_type>::value>>
    inline decltype(auto) combine (P && p, Q && q)
    {
        using V  = typename parser_traits<P>::value_type;
        using It = typename parser_traits<P>::range_type::iter_type;
        return parser<V, It>
        {
            .parse = [=](typename parser<V, It>::range_type const& r)
            {
                return fnk::append (fnk::eval (p.parse, r), fnk::eval (q.parse, r));
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
    inline decltype(auto) sequence (P && p, Q && q)
    {
        using V  = typename parser_traits<P>::value_type;
        using It = typename parser_traits<P>::range_type::iter_type;
        return parser<V, It>
        {
            .parse = [=](typename parser<V, It>::range_type const& r)
            {
                auto l1 = fnk::eval (p.parse, r);
                if (parser<V, It>::is_failure (l1.front()))
                    return l1;
                else {
                    auto l2 = fnk::eval (q.parse, parser<V, It>::result_range (l1.back()));
                    if (parser<V, It>::is_failure (l2.front()))
                       return l2;
                    else {
                        l1.splice (l1.cend(), l2); 
                        return l1; 
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
        typename = std::enable_if_t
            <std::is_same<typename parser_traits<P>::token_type, typename parser_traits<Q>::token_type>::value>>
    inline decltype(auto) ignorel (P && p, Q && q)
    {
        using PV = typename parser_traits<P>::value_type;
        using PI = typename parser_traits<P>::range_type::iter_type;
        using QV = typename parser_traits<Q>::value_type;
        using QR = typename parser_traits<Q>::result_type;
        return parser<QV, PI>
        {
            .parse = [=](typename parser<QV, PI>::range_type const& r)
            {
                auto l1 = fnk::eval (p.parse, r);
                if (parser<PV, PI>::is_failure (l1.front()))
                    return std::list<QR> { detail::failure{} };
                else 
                    return fnk::eval (q.parse, parser<PV, PI>::result_range (l1.back()));
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
            <std::is_same<typename parser_traits<P>::token_type, typename parser_traits<Q>::token_type>::value>>
    inline decltype(auto) ignorer (P && p, Q && q)
    {
        using PV = typename parser_traits<P>::value_type;
        using PT = typename parser_traits<P>::token_type;
        using PR = typename parser_traits<P>::result_type;
        using QV = typename parser_traits<Q>::value_type;
        using QT = typename parser_traits<Q>::token_type;
        return parser<PV, PT>
        {
            .parse = [=](range<PT> const& r)
            {
                auto l1 = fnk::eval (p.parse, r);
                if (parser<PV, PT>::is_failure (l1.front()))
                    return std::list<PR> { detail::failure{} };
                else {
                    auto l2 = fnk::eval (q.parse, parser<PV,PT>::result_range(l1.back()));
                    if (parser<QV, QT>::is_failure (l2.front()))
                        return std::list<PR> { detail::failure{} };
                    else {
                        auto l1_back = l1.pop_back();
                        l1.emplace_back (parser<PV,PT>::result_value(l1_back), parser<QV,QT>::result_range(l2.back()));
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
        using V  = typename parser_traits<P>::value_type;
        using It = typename parser_traits<P>::token_type;
        return parser<V, It>
        {
            .parse = [=](typename parser<V, It>::range_type const& r)
            {
                auto l = fnk::eval (p.parse, r);
                return parser<V, It>::is_failure (l.front()) ? fnk::eval (q.parse, r) : l;
            }
        };
    }

} // namespace parse

    template <typename V, typename It>
    struct functor<parse::parser<V, It>> : public default_functor<parse::parser, V, It>
    { 
        template <class F, typename U = typename type_support::function_traits<F>::return_type,
            typename = std::enable_if_t
                <std::is_convertible
                    <V, typename type_support::function_traits<F>::template argument<0>::type>::value>>
        static constexpr parse::parser<U, It> fmap (F && f, parse::parser<V, It> const& p)
        {
            return parse::parser<U, It>
            {
                .parse = [=](typename parse::parser<V, It>::range_type const& r)
                {
                    std::list
                        <typename parse::parser_traits
                            <typename parse::parser<V, It>::template rebind<U, It>>::result_type> out;
                    for (auto const& e : eval (p.parse, r))
                    {
                        if (parse::parser<V, It>::is_value_result (e))
                            type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair
                                        (eval (f, parse::parser<V, It>::result_value (e)),
                                         parse::parser<V, It>::result_range(e)));
                        else
                            type_support::container_traits<decltype(out)>::insert (out, parse::detail::failure{});
                    }
                    return out;
                }
            }; 
        }
    };

    template <typename V, typename It>
    struct functor<parse::parser<V, It> const> : public functor<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct functor<parse::parser<V, It> &> : public functor<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct functor<parse::parser<V, It> const&> : public functor<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct functor<parse::parser<V, It> &&> : public functor<parse::parser<V, It>> {};
    
    template <typename V, typename It>
    struct applicative_functor<parse::parser<V, It>> : public default_applicative_functor<parse::parser<V, It>>
    {
        static inline constexpr decltype(auto) pure (V && v)
        {
            return unit (std::forward<V>(v));
        }

        template <class F, typename U,
            typename = std::enable_if_t<parse::is_parser_instance<F>::value>,
            typename = std::enable_if_t<parse::is_parser_instance<U>::value>>
        static inline constexpr decltype(auto) apply (F const& fs, U const& us)
        {
            using W = typename parse::parser_traits<F>::value_type;
            using S = typename parse::parser_traits<F>::token_type; 
            return parse::parser<W, S>
            {
                .parse = [=](parse::range<S> const& r)
                {
                    std::list<typename parse::parser<W,S>::result_type> out;
                    for (auto const& e1 : eval (fs.parse, r))
                    {
                        if (std::decay_t<F>::is_value_result (e1)) {
                            for (auto const& e2 : eval (us.parse, std::decay_t<F>::result_range (e1)))
                            {
                                if (std::decay_t<U>::is_value_result (e2))
                                    type_support::container_traits<decltype(out)>::insert
                                        (out, 
                                         std::make_pair (eval (std::decay_t<F>::result_value(e1), std::decay_t<U>::result_value(e2)),
                                                         std::decay_t<U>::result_range(e2)));
                                else
                                    type_support::container_traits<decltype(out)>::insert (out, parse::detail::failure{});
                            }
                        } else 
                            type_support::container_traits<decltype(out)>::insert (out, parse::detail::failure{});                
                    }
                    return out;
                }
            };
        }
    };

    template <typename V, typename It>
    struct applicative_functor<parse::parser<V, It> const> : public applicative_functor<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct applicative_functor<parse::parser<V, It> &> : public applicative_functor<parse::parser<V, It>> {};   
    template <typename V, typename It>
    struct applicative_functor<parse::parser<V, It> const&> : public applicative_functor<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct applicative_functor<parse::parser<V, It> &&> : public applicative_functor<parse::parser<V, It>> {};
    
    template <typename V, typename It>
    struct alternative<parse::parser<V, It>> : public default_alternative<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct alternative<parse::parser<V, It> const> : public alternative<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct alternative<parse::parser<V, It> &> : public alternative<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct alternative<parse::parser<V, It> const&> : public alternative<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct alternative<parse::parser<V, It> &&> : public alternative<parse::parser<V, It>> {};
    
    template <typename V, typename It>
    struct monad<parse::parser<V, It>> : public default_monad<parse::parser<V, It>>
    {
        template <class F, typename P,
            typename = std::enable_if_t<std::is_same<std::decay_t<P>, parse::parser<V, It>>::value>>
        static inline constexpr decltype(auto) mbind (P && p, F && f)
        {
            return bind (std::forward<P>(p), std::forward<F>(f));
        }
    };

    template <typename V, typename It>
    struct monad<parse::parser<V, It> const> : public monad<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct monad<parse::parser<V, It> &> : public monad<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct monad<parse::parser<V, It> const&> : public monad<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct monad<parse::parser<V, It> &&> : public monad<parse::parser<V, It>> {};
    
    template <typename V, typename It>
    struct monoid<parse::parser<V, It>> 
    {
        static inline constexpr decltype(auto) unity (void) { return parse::fail<V, It>; } 
   
        template <typename P, typename Q,
            typename = std::enable_if_t<parse::is_parser_instance<P>::value>,
            typename = std::enable_if_t<parse::is_parser_instance<Q>::value>,
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

    template <typename V, typename It>
    struct monoid<parse::parser<V, It> const> : public monoid<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct monoid<parse::parser<V, It> &> : public monoid<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct monoid<parse::parser<V, It> const&> : public monoid<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct monoid<parse::parser<V, It> &&> : public monoid<parse::parser<V, It>> {};
    
    template <typename V, typename It>
    struct additive_monad<parse::parser<V, It> const> : public additive_monad<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct additive_monad<parse::parser<V, It> &> : public additive_monad<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct additive_monad<parse::parser<V, It> const&> : public additive_monad<parse::parser<V, It>> {};
    template <typename V, typename It>
    struct additive_monad<parse::parser<V, It> &&> : public additive_monad<parse::parser<V, It>> {};

namespace parse
{
    template <typename V, typename It>
    auto item = parser<V, It>
    {
        .parse = [](typename parser<V, It>::range_type const& r)
        {
            return r.is_empty() ? std::list<typename parser<V, It>::result_type>{ detail::failure{"expected item"} }
                                : std::list<typename parser<V, It>::result_type>{ std::make_pair (r.head(), r.tail()) }; 
        }
    };

    //
    // One or more successful parses
    //
    template <typename V, typename It>
    inline decltype(auto) some (parser<V, It> const& p)
    {
        return parser<V, It>
        {
            .parse = [=](typename parser<V, It>::range_type const& r)
            {
                auto l1 = eval (p.parse, r);
                if (parser<V, It>::is_failure (l1.front()))
                    return std::list<typename parser<V, It>::result_type>{ detail::failure{"expected at least one result"} };
                else {
                    while (true)
                    {
                        auto l2 = eval (p.parse, parser<V, It>::result_range(l1.back()));
                        if (parser<V, It>::is_failure (l2.front()))
                            return l1;
                        else
                            l1.splice (l1.cend(), l2);
                    }
                } 
            }
        }; 
    }

    //
    // Zero or more successful parses
    //
    template <typename V, typename It>
    inline decltype(auto) many (parser<V, It> const& p)
    {
        return parser<V, It>
        {
            .parse = [=](typename parser<V, It>::range_type const& r)
            {
                auto l1 = eval (p.parse, r);
                if (parser<V, It>::is_failure (l1.front()))
                    return std::list<typename parser<V, It>::result_type>{ std::make_pair (detail::empty_result<V>{}, r) };
                else {
                    while (true)
                    {
                        auto l2 = eval (p.parse, parser<V, It>::result_range(l1.back()));
                        if (parser<V, It>::is_failure (l2.front()))
                            return l1;
                        else
                            l1.splice (l1.cend(), l2);
                    }
                }
            }
        }; 
    }

    template <typename It, typename P,
        typename = std::enable_if_t<std::is_same<typename type_support::function_traits<P>::return_type, bool>::value>,
        typename = std::enable_if_t
            <std::is_convertible<typename type_support::function_traits<P>::template argument<0>::type, 
                                 typename std::iterator_traits<It>::value_type>::value>>
    inline constexpr decltype(auto) satisfy (P && p, std::string const& description = std::string(""))
    {
        using T = std::decay_t<typename type_support::function_traits<P>::template argument<0>::type>;
        using OT = std::list<typename parser<T, It>::result_type>;
        
        return parser<T, It>
        {
            .parse = [=](typename parser<T, It>::range_type const& r)
            {
                if (r.is_empty())
                    return OT { detail::failure{"no remaining tokens to parse"} };
                else {
                    if (eval (p, r.head()))
                        return OT { std::make_pair (r.head(), r.tail()) };
                    else
                        return OT { detail::failure{std::string("predicate '") + description + std::string("' failed")} };
                }
            } 
        };
    }
} // namespace parse
} // namespace fnk

#endif // ifndef PARSER_HPP

