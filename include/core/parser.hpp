//
// A tiny parser library using `funktional`
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef PARSER_HPP
#define PARSER_HPP

#include <algorithm>
#include <type_traits>
#include <functional>

#include "accumulator.hpp"
#include "range.hpp"
#include "result_type.hpp"

#include "funktional/include/algebraic.hpp"
#include "funktional/include/concat.hpp"
#include "funktional/include/filterable.hpp"
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
    enum class status : bool
    {
        SUCCESS = true,
        FAILURE = false
    };

    template <typename It, typename V, typename R = range<It>> 
    struct parser
    {
        using type        = parser<It, V, R>;
        using iter_type   = It;
        using range_type  = R;
        using token_type  = 
            typename std::iterator_traits<iter_type>::value_type;
        using value_type  = V;
        using fail_type   = failure;
        using empty_type  = empty_result <V>;
        using value_result_type = value_result <V>;
        using result_type       = parse_result <V>;
        using accumulator_type  = accumulator <It, V, R>;

        template <typename I, typename U, typename S = range<I>>
        using rebind = parser<I, U, S>;

        std::string const description;
        std::function<accumulator_type & (accumulator_type &)> const parse;
    };

    template <typename T>
    struct is_parser_instance : public std::false_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R>>
        : public std::true_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R>&>
        : public std::true_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R> const>
        : public std::true_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R> const&>
        : public std::true_type {};

    template <typename It, typename V, typename R>
    struct is_parser_instance<parser<It, V, R> &&>
        : public std::true_type {};

    template <typename P,
              typename = std::enable_if_t<is_parser_instance<P>::value>>
    struct parser_traits
    {
        using type        = std::decay_t<P>;
        using iter_type   = typename type::iter_type;
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
    inline decltype(auto) override_description
        (parser<It, V, R> const& p, std::string const& new_description)
    {
        return parser<It, V, R>
        {
            .description = new_description,
            .parse = p.parse
        };
    }

    template <typename It, typename V, typename R>
    inline decltype(auto) parse
        (parser<It, V, R> const& p,
         typename parser<It, V, R>::range_type const& r)
    {
        typename parser<It, V, R>::accumulator_type acc {empty<V>{}, r};
        auto stat (p.parse (acc));
        return std::make_pair (stat, acc);
    }

    template <typename A>
    inline bool parse_success (A const& acc, std::size_t const n = 0)
    {
        return is_success (acc.result (n));
    }
 
    template <typename A>
    inline decltype(auto) torange (A const& acc, std::size_t const n = 0)
    {
        return acc.range (n);
    }
 
    template <typename A>
    inline decltype(auto) torange_tail (A const& acc, std::size_t const n = 0)
    {
        return acc.range_tail (n);
    }
 
    template <typename A>
    inline decltype(auto) toresult (A const& acc, std::size_t const n = 0)
    {
        return acc.result (n);
    }
    
    template <typename A>
    inline decltype(auto) toresult_value (A const& acc, std::size_t const n = 0)
    {
        return result_value (acc.result (n));
    }
    
    template <typename A>
    inline decltype(auto) toresult_failure (A const& acc, std::size_t const n = 0)
    {
        return result_failure (acc.result (n));
    }
/* 
    template <typename V, typename A>
    inline decltype(auto) values (A const& acc, std::size_t const n = 0)
    {
        using ResT = typename accumulator_traits<A>::result_type;
        using RngT = typename accumulator_traits<A>::range_type;

        auto vs (fnk::map<decltype(result_value<V>)>
            (result_value<V>,
             fnk::filter<decltype(is_value<V>)>
                (is_value<V>,
                 fnk::map
                    ([](std::pair<ResT, RngT> const& r) { return r.first; }, acc.data ()))));
        std::reverse (vs.begin(), vs.end());
        return vs;
    }
    
    template <typename V, typename A>
    inline decltype(auto) values (std::pair<status, A> const& pres)
    {
        using ResT = typename accumulator_traits<A>::result_type;
        using RngT = typename accumulator_traits<A>::range_type;
       
        return fnk::map<decltype(result_value<V>)>
            (result_value<V>,
             fnk::filter<decltype(is_value<V>)>
                (is_value<V>,
                 fnk::map
                    ([](std::pair<ResT, RngT> const& r) { return r.first; }, pres.second.data ())));
    }
    */
} // namespace core
} // namespace rpc

namespace fnk
{
/*    template <typename It, typename V, typename R>
    struct functor<rpc::core::parser<It, V, R>> : public default_functor<rpc::core::parser, It, V, R>
    { 
        template <class F, typename U = typename type_support::function_traits<F>::return_type,
            typename = std::enable_if_t
                <std::is_convertible
                    <V, typename type_support::function_traits<F>::template argument<0>::type>::value>>
        static inline rpc::core::parser<It, U, R> fmap (F && f, rpc::core::parser<It, V, R> const& p)
        {
            return rpc::core::parser<It, U, R>
            {
                .description = "[" + p.description + " //fmap// " + fnk::utility::format_function_type<F>() + "]",
                .parse = [=](typename rpc::core::parser<It, U, R>::accumulator_type & acc)
                {
                    typename rpc::core::parser<It, V, R>::accumulator_type tmp
                        { rpc::core::empty<V>{}, acc.range() };
                    
                    auto res (p.parse (tmp));
                    if (static_cast<bool>(res)) {
                        acc.insert (fnk::eval (f, result_value (tmp.result ())), tmp.range());
                        return rpc::core::status::SUCCESS;
                    } else {
                        acc.insert (rpc::core::failure{rpc::core::result_failure (tmp.result ())}, tmp.range());
                        return rpc::core::status::FAILURE;
                    }
                }
            };
        }
    };

    template <typename It, typename V, typename R>
    struct functor<rpc::core::parser<It, V, R> const> : public functor<rpc::core::parser<It, V, R>> {};
    template <typename It, typename V, typename R>
    struct functor<rpc::core::parser<It, V, R> &> : public functor<rpc::core::parser<It, V, R>> {};
    template <typename It, typename V, typename R>
    struct functor<rpc::core::parser<It, V, R> const&> : public functor<rpc::core::parser<It, V, R>> {};
    template <typename It, typename V, typename R>
    struct functor<rpc::core::parser<It, V, R> &&> : public functor<rpc::core::parser<It, V, R>> {};
    
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
            using R = typename type_support::function_traits<F>::return_type;
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

