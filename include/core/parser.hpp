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

#include "range.hpp"

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

namespace rpc
{
namespace core
{
namespace detail
{
    enum
    {
        PARSE_RESULT       = 0,
        PARSE_EMPTY_RESULT = 1,
        PARSE_FAILURE      = 2
    };
} // namespace detail
 
    struct failure
    {
        failure (void) : msg("failure") {}
        failure (std::string const& s) : msg(s) {}
        failure (std::string && s) : msg(s) {}
        failure (failure const& f) : msg(f.msg) {}
    
        std::string const msg;
    };

    template <typename It>
    struct empty_result {};

    template <typename IterT, typename ValT, typename TokenT = typename std::iterator_traits<IterT>::value_type>
    struct parser
    {
        using range_type  = range<IterT>;
        using value_type  = ValT;
        using empty_type  = empty_result<ValT>; 
        using token_type  = TokenT;
        using result_type = fnk::adt<std::pair<value_type, range_type>, std::pair<empty_type, range_type>, failure>;

        template <typename S, typename U>
        using rebind = parser<S, U>;

        using parse_type = std::function<std::list<result_type>(range_type const&)>;
        
        parse_type parse;
      
        inline decltype(auto) operator() (range_type const& r)
        {
            return parse (r); 
        }

        static inline decltype(auto) is_result (result_type const& r)
        {
            return r.type_index() == detail::PARSE_RESULT || r.type_index() == detail::PARSE_EMPTY_RESULT;
        }
        
        static inline decltype(auto) is_value_result (result_type const& r)
        {
            return r.type_index() == detail::PARSE_RESULT;
        }
        
        static inline decltype(auto) is_empty_result (result_type const& r)
        {
            return r.type_index() == detail::PARSE_EMPTY_RESULT;
        }

        static inline decltype(auto) is_failure (result_type const& r)
        {
            return r.type_index() == detail::PARSE_FAILURE;
        }

        static inline decltype(auto) result (result_type const& r)
        {
            return r.template value<std::pair<value_type, range_type>>();
        }
        
        static inline decltype(auto) empty_result (result_type const& r)
        {
            return r.template value<std::pair<empty_type, range_type>>();
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
           return r.template value<failure>();
        }

        static inline decltype(auto) values (std::list<result_type> const& l)
        {
            std::list<ValT> out;
            for (auto const& e : l)
                if (is_value_result(e))
                    fnk::type_support::container_traits<decltype(out)>::insert (out, result_value (e));
            return out;
        }
 
        static inline decltype(auto) failure_messages (std::list<result_type> const& l)
        {
            std::list<failure> out;
            for (auto const& e : l)
                if (is_failure(e))
                    fnk::type_support::container_traits<decltype(out)>::insert (out, failure_message (e));
            return out;
        }
    };

    template <typename It, typename V>
    struct parser<It const&, V const> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It const, V const&> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It&, V const> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It const, V&> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It const&, V> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It, V const&> : public parser<It, V> {};
   
    template <typename It, typename V>
    struct parser<It&, V> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It, V&> : public parser<It, V> {}; 
    
    template <typename It, typename V>
    struct parser<It&&, V> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It, V&&> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It&&, V const> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It const, V&&> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It&&, V const&> : public parser<It, V> {};
    
    template <typename It, typename V>
    struct parser<It const&, V&&> : public parser<It, V> {};

    template <typename V>
    struct is_parser_instance : public std::false_type {};

    template <typename It, typename V>
    struct is_parser_instance<parser<It, V>> : public std::true_type {};

    template <typename It, typename V>
    struct is_parser_instance<parser<It, V>&> : public std::true_type {};

    template <typename It, typename V>
    struct is_parser_instance<parser<It, V> const> : public std::true_type {};

    template <typename It, typename V>
    struct is_parser_instance<parser<It, V> const&> : public std::true_type {};

    template <typename It, typename V>
    struct is_parser_instance<parser<It, V> &&> : public std::true_type {};

    template <typename P,
        typename = std::enable_if_t<is_parser_instance<P>::value>>
    struct parser_traits
    {
        using range_type  = typename std::decay_t<P>::range_type;
        using value_type  = typename std::decay_t<P>::value_type;
        using token_type  = typename std::decay_t<P>::token_type;
        using result_type = typename std::decay_t<P>::result_type;

        using parse_type = typename std::decay_t<P>::parse_type;
        
        template <typename S, typename U>
        using rebind = typename std::decay_t<P>::template rebind<S, U>;
    };
} // namespace core
} // namespace rpc

namespace fnk
{
    template <typename It, typename V>
    struct functor<rpc::core::parser<It, V>> : public default_functor<rpc::core::parser, It, V>
    { 
        template <class F, typename U = typename type_support::function_traits<F>::return_type,
            typename = std::enable_if_t
                <std::is_convertible
                    <V, typename type_support::function_traits<F>::template argument<0>::type>::value>>
        static constexpr rpc::core::parser<It, U> fmap (F && f, rpc::core::parser<It, V> const& p)
        {
            return rpc::core::parser<It, U>
            {
                .parse = [=](typename rpc::core::parser<It, U>::range_type const& r)
                {
                    std::list
                        <typename rpc::core::parser_traits
                            <typename rpc::core::parser<It, V>::template rebind<It, U>>::result_type> out;
                    for (auto const& e : eval (p.parse, r))
                    {
                        if (rpc::core::parser<It, V>::is_value_result (e))
                            type_support::container_traits<decltype(out)>::insert
                                (out, std::make_pair
                                        (eval (f, rpc::core::parser<It, V>::result_value (e)),
                                         rpc::core::parser<It, V>::result_range(e)));
                        else
                            type_support::container_traits<decltype(out)>::insert (out, rpc::core::failure{});
                    }
                    return out;
                }
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
        static inline constexpr decltype(auto) pure (It && v)
        {
            return rpc::core::parser<It, V>
            {
                .parse = [=](typename rpc::core::parser<It, V>::range_type const& r)
                {
                    return std::list<typename rpc::core::parser<It, V>::result_type> { std::make_pair (v, r) };
                }
            };
        }

        template <class F, typename U,
            typename = std::enable_if_t<rpc::core::is_parser_instance<F>::value>,
            typename = std::enable_if_t<rpc::core::is_parser_instance<U>::value>>
        static inline constexpr decltype(auto) apply (F const& fs, U const& us)
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
                            for (auto const& e2 : eval (us.parse, std::decay_t<F>::result_range (e1)))
                            {
                                if (std::decay_t<U>::is_value_result (e2))
                                    type_support::container_traits<decltype(out)>::insert
                                        (out, 
                                         std::make_pair
                                            (eval (std::decay_t<F>::result_value(e1), std::decay_t<U>::result_value(e2)),
                                             std::decay_t<U>::result_range(e2)));
                                else
                                    type_support::container_traits<decltype(out)>::insert (out, rpc::core::failure{});
                            }
                        } else 
                            type_support::container_traits<decltype(out)>::insert (out, rpc::core::failure{});                
                    }
                    return out;
                }
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
        static inline constexpr decltype(auto) mbind (P && p, F && f)
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
                }
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
        static inline constexpr decltype(auto) unity (void)
        {
            return rpc::core::parser<It, V>
            {
                .parse = [](typename rpc::core::parser<It, V>::range_type const& /*r*/)
                {
                    return std::list<typename rpc::core::parser<It, V>::result_type> { rpc::core::failure{} };
                }
            };
        } 
   
        template <typename P, typename Q,
            typename = std::enable_if_t<rpc::core::is_parser_instance<P>::value>,
            typename = std::enable_if_t<rpc::core::is_parser_instance<Q>::value>,
            typename = std::enable_if_t
                <std::is_base_of<rpc::core::parser<It, V>, std::decay_t<P>>::value>,
            typename = std::enable_if_t
                <std::is_base_of<rpc::core::parser<It, V>, std::decay_t<Q>>::value>>
        static inline constexpr decltype(auto) append (P && p, Q && q)
        {
            return rpc::core::parser<It, V>
            {
                .parse = [=](typename rpc::core::parser<It, V>::range_type const& r)
                {
                    auto l = fnk::eval (std::forward<P>(p).parse, r);
                    return rpc::core::parser<It, V>::is_failure (l.front()) ? fnk::eval (std::forward<Q>(q).parse, r) : l;
                }
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
} // namespace fnk

#endif // ifndef PARSER_HPP

