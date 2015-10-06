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

#include "accumulator.hpp"
#include "range.hpp"
#include "result_type.hpp"

#include "funktional/include/algebraic.hpp"
#include "funktional/include/concat.hpp"
#include "funktional/include/mappable.hpp"

#include "funktional/include/alternative.hpp"
#include "funktional/include/functor.hpp"
#include "funktional/include/applicative_functor.hpp"
#include "funktional/include/monad.hpp"
#include "funktional/include/additive_monad.hpp"
#include "funktional/include/monoid.hpp"

#include "funktional/include/type_support/container_traits.hpp"
#include "funktional/include/type_support/function_traits.hpp"
#include "funktional/include/utility/type_utils.hpp"

namespace rpc
{
namespace core
{
    template <typename It, typename V, typename R = range<It>> 
    struct parser;

namespace detail
{
    //
    // Bottoms out a call chain and returns the accumulator.
    //
    template <typename It, typename V, typename R>
    parser<It, V, R> bot
    {
        .description = "[bot]",
        .parse = [] (typename parser<It, V, R>::accumulator_type & acc,
                     parser<It, V, R> const& /*succ*/,
                     parser<It, V, R> const& /*next*/)
        {
            return acc.insert (parse_result<V> {detail::botr<V>{}, acc.range()});
        }
    };
    
    //
    // Tops the call chain.
    //
    template <typename It, typename V, typename R>
    parser<It, V, R> top
    {
        .description = "[top]",
        .parse = [] (typename parser<It, V, R>::accumulator_type & acc,
                     parser<It, V, R> const& succ = bot<It, V, R>,
                     parser<It, V, R> const& next = bot<It, V, R>)
        {
            return acc.insert (parse_result<V> {detail::topr<V>{}, acc.range()});
        }
    };
} // namespace detail

    //
    // Continues a parse chain when there is no available success continuation.
    // This eventually forces a top-out.
    //
    template <typename It, typename V, typename R>
    inline decltype(auto) continuation (parser<It, V, R> const& p,
                                        typename parser<It, V, R>::accumulator_type & acc)
    {
        return is_success (acc.result()) ? p.parse (detail::top<It, V, R>, detail::top<It, V, R>, acc)
                                         : acc;
    }

    //
    // Applies parser to input range.
    //
    template <typename It, typename V, typename R>
    inline decltype(auto) apply (parser<It, V, R> const& p, typename parser<It, V, R>::range_type const& r)
    {
    }

    template <typename It, typename V, typename R> 
    struct parser
    {
        using type        = parser<It, V, R>;
        using range_type  = R;
        using token_type  = typename range_traits <range_type>::token_type;
        using value_type  = V;
        using fail_type   = failure_result;
        using empty_type  = empty_result <V>;
        using value_result_type = value_result <V>;
        using result_type       = parse_result <V>;
        using accumulator_type  = accumulator <It, V, R>;

        template <typename I, typename U, typename S = range<I>>
        using rebind = parser<I, U, S>;

        std::string const description;
        std::function<accumulator_type (accumulator_type const&, type const&, type const&)> const parse;
    };

    template <typename T>
    struct is_parser_instance : public std::false_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R>> : public std::true_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R>&> : public std::true_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R> const> : public std::true_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R> const&> : public std::true_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R> &&> : public std::true_type {};

    template <typename P, typename = std::enable_if_t<is_parser_instance<P>::value>>
    struct parser_traits
    {
        using type        = P;
        using range_type  = typename type::range_type;
        using token_type  = typename range_traits <range_type>::token_type;
        using value_type  = typename type::value_type;
        using fail_type   = typename type::fail_type;
        using empty_type  = typename type::empty_type;
        using value_result_type = typename type::value_result_type;
        using result_type       = typename type::result_type;
        using accumulator_type  = typename type::accumulator_type;

        template <typename I, typename U, typename R = range<I>>
        using rebind = typename std::decay_t<P>::template rebind<I, U, R>;
    };
    
    template <typename It, typename V, typename R>
    inline decltype(auto) override_description (parser<It, V, R> const& p, std::string const& new_description)
    {
        return parser<It, V, R>
        {
            .description = new_description,
            .parse = p.parse
        };
    }
} // namespace core
} // namespace rpc

namespace fnk
{
/* to be rewritten later involving the continuations
    template <typename It, typename V>
    struct functor<rpc::core::parser<It, V>> : public default_functor<rpc::core::parser, It, V>
    { 
        template <class F, typename U = typename type_support::function_traits<F>::return_type,
            typename = std::enable_if_t
                <std::is_convertible
                    <V, typename type_support::function_traits<F>::template argument<0>::type>::value>>
        static inline rpc::core::parser<It, U> fmap (F && f, rpc::core::parser<It, V> const& p)
        {
            return rpc::core::parser<It, U>
            {
                .parse = [=](typename rpc::core::parser<It, U>::range_type const& r)
                {
                    std::list<typename rpc::core::parser<It, U>::result_type> out;
                    for (auto const& e : eval (p.parse, r))
                    {
                        if (rpc::core::parser<It, V>::is_value_result (e))
                            type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair
                                        (eval (f, rpc::core::parser<It, V>::result_value (e)),
                                         rpc::core::parser<It, V>::result_range(e)));
                        else if (rpc::core::parser<It, V>::is_empty_result (e))
                            type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair
                                        (rpc::core::empty_result<U>{},
                                         rpc::core::parser<It, V>::result_range(e)));
                        else
                            type_support::container_traits<decltype(out)>::insert (out, rpc::core::failure{});
                    }
                    return out;
                },
                .description
                    = std::string("[")
                    + p.description
                    + std::string(" `fmap` ")
                    + fnk::utility::format_function_type<F>()
                    + std::string("]")
            }; 
        }
    };

    template <typename It, typename V>
    struct functor<rpc::core::parser<It, V> const> : public functor<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct functor<rpc::core::parser<It, V> &> : public functor<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct functor<rpc::core::parser<It, V> const&> : public functor<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct functor<rpc::core::parser<It, V> &&> : public functor<rpc::core::parser<It, V>> {};
    
    template <typename It, typename V>
    struct applicative_functor<rpc::core::parser<It, V>> : public default_applicative_functor<rpc::core::parser<It, V>>
    {
        static inline decltype(auto) pure (V && v)
        {
            return rpc::core::parser<It, V>
            {
                .parse = [=](typename rpc::core::parser<It, V>::range_type const& r)
                {
                    return std::list<typename rpc::core::parser<It, V>::result_type> { std::make_pair (v, r) };
                },
                .description
                    = std::string("[pure: ")
                    + utility::to_string<V> (v)
                    + std::string(" :: ")
                    + fnk::utility::type_name<V>::name()
                    + std::string("]")
            };
        }

        template <class F, typename U,
            typename = std::enable_if_t<rpc::core::is_parser_instance<F>::value>,
            typename = std::enable_if_t<rpc::core::is_parser_instance<U>::value>>
        static inline decltype(auto) apply (F const& fs, U const& us)
        {
            using S = typename rpc::core::parser_traits<F>::range_type::iter_type; 
            using W = typename rpc::core::parser_traits<F>::value_type;
            return rpc::core::parser<S, W>
            {
                .parse = [=](typename rpc::core::parser<S, W>::range_type const& r)
                {
                    std::list<typename rpc::core::parser<S, W>::result_type> out;
                    for (auto const& e1 : eval (fs.parse, r))
                    {
                        if (std::decay_t<F>::is_value_result (e1)) {
                            for (auto const& e2 : eval (us.parse, fs.result_range (e1)))
                            {
                                if (us.is_value_result (e2))
                                    type_support::container_traits<decltype(out)>::insert
                                        (out, 
                                         std::make_pair
                                            (eval (fs.result_value(e1), us.result_value(e2)),
                                             us.result_range(e2)));
                                else
                                    type_support::container_traits<decltype(out)>::insert (out, rpc::core::failure{});
                            }
                        } else 
                            type_support::container_traits<decltype(out)>::insert (out, rpc::core::failure{});                
                    }
                    return out;
                },
                .description
                    = std::string("[")
                    + fs.description
                    + std::string(" `apply` ")
                    + us.description
                    + std::string("]")
            };
        }
    };

    template <typename It, typename V>
    struct applicative_functor<rpc::core::parser<It, V> const> : public applicative_functor<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct applicative_functor<rpc::core::parser<It, V> &> : public applicative_functor<rpc::core::parser<It, V>> {};   
    template <typename It, typename V>
    struct applicative_functor<rpc::core::parser<It, V> const&> : public applicative_functor<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct applicative_functor<rpc::core::parser<It, V> &&> : public applicative_functor<rpc::core::parser<It, V>> {};
    
    template <typename It, typename V>
    struct alternative<rpc::core::parser<It, V>> : public default_alternative<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct alternative<rpc::core::parser<It, V> const> : public alternative<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct alternative<rpc::core::parser<It, V> &> : public alternative<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct alternative<rpc::core::parser<It, V> const&> : public alternative<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct alternative<rpc::core::parser<It, V> &&> : public alternative<rpc::core::parser<It, V>> {};
    
    template <typename It, typename V>
    struct monad<rpc::core::parser<It, V>> : public default_monad<rpc::core::parser<It, V>>
    {
        template <class F, typename P,
            typename = std::enable_if_t<std::is_same<std::decay_t<P>, rpc::core::parser<It, V>>::value>>
        static inline decltype(auto) mbind (P && p, F && f)
        {
            using R = typename type_support::function_traits<F>::result_type;
            return rpc::core::parser<It, typename R::value_type>
            {
                .parse = [=](typename rpc::core::parser_traits<R>::range_type const& r)
                {
                    return concat
                        (map
                            ([=](typename rpc::core::parser<It, V>::result_type const& e)
                             {
                                if (rpc::core::parser<It, V>::is_value_result (e))
                                    return eval
                                        (eval (f, rpc::core::parser<It, V>::result_value (e)).parse,
                                         rpc::core::parser<It, V>::result_range (e));
                                else
                                    return std::list<typename rpc::core::parser_traits<R>::result_type>{ rpc::core::failure{} };
                             },
                            eval (std::forward<P>(p).parse, r)));
                },
                .description
                    = std::string("[")
                    + p.description
                    + std::string(" `mbind` ")
                    + fnk::utility::format_function_type<F>()
                    + std::string("]")
            };
        }
    };

    template <typename It, typename V>
    struct monad<rpc::core::parser<It, V> const> : public monad<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct monad<rpc::core::parser<It, V> &> : public monad<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct monad<rpc::core::parser<It, V> const&> : public monad<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct monad<rpc::core::parser<It, V> &&> : public monad<rpc::core::parser<It, V>> {};
    
    template <typename It, typename V>
    struct monoid<rpc::core::parser<It, V>> 
    {
        static inline decltype(auto) unity (void)
        {
            return rpc::core::parser<It, V>
            {
                .parse = [](typename rpc::core::parser<It, V>::range_type const& \r*\/)
                {
                    return std::list<typename rpc::core::parser<It, V>::result_type> { rpc::core::failure{} };
                },
                .description = "[failure]"
            };
        } 
   
        template <typename P, typename Q,
            typename = std::enable_if_t<rpc::core::is_parser_instance<P>::value>,
            typename = std::enable_if_t<rpc::core::is_parser_instance<Q>::value>,
            typename = std::enable_if_t
                <std::is_base_of<rpc::core::parser<It, V>, std::decay_t<P>>::value>,
            typename = std::enable_if_t
                <std::is_base_of<rpc::core::parser<It, V>, std::decay_t<Q>>::value>>
        static inline decltype(auto) append (P && p, Q && q)
        {
            return rpc::core::parser<It, V>
            {
                .parse = [=](typename rpc::core::parser<It, V>::range_type const& r)
                {
                    auto l = fnk::eval (std::forward<P>(p).parse, r);
                    return rpc::core::parser<It, V>::is_failure (l.front()) ? fnk::eval (std::forward<Q>(q).parse, r) : l;
                },
                .description
                    = std::string("[")
                    + p.description
                    + std::string(" `or` ")
                    + q.description
                    + std::string("]")
            };
        }

        struct is_monoid_instance : public std::true_type {};
    };

    template <typename It, typename V>
    struct monoid<rpc::core::parser<It, V> const> : public monoid<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct monoid<rpc::core::parser<It, V> &> : public monoid<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct monoid<rpc::core::parser<It, V> const&> : public monoid<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct monoid<rpc::core::parser<It, V> &&> : public monoid<rpc::core::parser<It, V>> {};
    
    template <typename It, typename V>
    struct additive_monad<rpc::core::parser<It, V> const> : public additive_monad<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct additive_monad<rpc::core::parser<It, V> &> : public additive_monad<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct additive_monad<rpc::core::parser<It, V> const&> : public additive_monad<rpc::core::parser<It, V>> {};
    template <typename It, typename V>
    struct additive_monad<rpc::core::parser<It, V> &&> : public additive_monad<rpc::core::parser<It, V>> {};
*/
} // namespace fnk

#endif // ifndef PARSER_HPP

