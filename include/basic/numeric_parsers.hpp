//
// Parsers for recognizing numeric types
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef NUMERIC_PARSERS_HPP
#define NUMERIC_PARSERS_HPP

#include <iterator>
#include <locale>
#include <string>

#include "core/parser.hpp"
#include "core/combinators.hpp"

#include "atom_parsers.hpp"
#include "char_parsers.hpp"

#include "funktional/include/eval.hpp"
#include "funktional/include/functor.hpp"

namespace rpc
{
namespace basic
{
namespace detail
{
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>> natural_str =
    {
        .parse = [](typename rpc::core::parser<It, std::basic_string<CharT>>::range_type const& r)
        {
            using P  = std::decay_t<decltype(digits<It, CharT>)>;
            using OT = std::list<typename rpc::core::parser<It, std::basic_string<CharT>>::result_type>;

            auto preres = fnk::eval (digits<It, CharT>.parse, r);
            if (P::is_value_result (preres.front())) {
                auto vs = P::values (preres);
                return OT
                {
                    std::make_pair
                        (fnk::eval ([](std::list<typename P::value_type> const& cs) -> std::basic_string<CharT>&&
                                    {
                                        std::basic_string<CharT> a;
                                        a.reserve (cs.size());
                                        for (auto const& c : cs)
                                            a.push_back (c);
                                        return std::move(a);
                                    },
                                    vs),
                         P::result_range (preres.back()))
                };
            } else
                return OT { rpc::core::failure{"expected a (natural) number"} };
        }
    };

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>> plus_natural_str =
        rpc::core::ignorel (token<It, CharT> (static_cast<CharT>('+')), natural_str<It, CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>> minus_natural_str 
    {
        .parse = [](typename rpc::core::parser<It, std::basic_string<CharT>>::range_type const& r)
        {
            using P = std::decay_t<decltype(character<It, CharT> (static_cast<CharT>('-')))>;
            using R = typename rpc::core::parser<It, std::basic_string<CharT>>::result_type;

            auto l1 = fnk::eval (character<It, CharT> (static_cast<CharT>('-')).parse, r);
            if (not P::is_result (l1.front()))
                return std::list<R>{ rpc::core::failure{"expected negative number"} };
            else {
                auto l2 = fnk::eval (natural_str<It, CharT>.parse, P::result_range (l1.back()));
                if (not natural_str<It, CharT>.is_result (l2.front()))
                    return l2;
                else {
                    auto out (std::basic_string<CharT> (1, P::result_value (l1.front())));
                    out += natural_str<It, CharT>.result_value (l2.front());
                    return std::list<R>{ std::make_pair (out, natural_str<It, CharT>.result_range (l2.back())) };
                }
            } 
        }
    };

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto number_str = rpc::core::option (natural_str<It,CharT>, minus_natural_str<It,CharT>, plus_natural_str<It,CharT>);
} // namespace detail
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto todigit = fnk::functor<rpc::core::parser<It, CharT>>::fmap
        ([](CharT c) -> IntT { return (c - static_cast<CharT>('0')); }, basic::digit<It, CharT>); 
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto todigits = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, todigit<It, CharT, IntT, base>));
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto towdigit = fnk::functor<rpc::core::parser<It, CharT>>::fmap
        ([](CharT c) -> IntT { return (c - static_cast<CharT>('0')); }, basic::wdigit<It, CharT>); 
   
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto towdigits = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, towdigit<It, CharT, IntT, base>)); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto natural = fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
        ([](std::basic_string<CharT> const& s) -> unsigned long { return std::stoul(s, nullptr, base); },
         detail::natural_str<It,CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto naturals = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, natural<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto naturall = fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
        ([](std::basic_string<CharT> const& s) -> unsigned long long { return std::stoull(s,nullptr,base); },
         detail::natural_str<It,CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto naturalls = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, naturall<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integer = fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
        ([](std::basic_string<CharT> const& s) -> int { return std::stoi(s, nullptr, base); },
         detail::number_str<It, CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integers = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, integer<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integerl = fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
        ([](std::basic_string<CharT> const& s) -> long { return std::stol(s, nullptr, base); },
         detail::number_str<It, CharT>);
 
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integerls = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, integerl<It, CharT, base>));
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integerll = fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
        ([](std::basic_string<CharT> const& s) -> long long { return std::stoll(s, nullptr, base); },
         detail::number_str<It, CharT>);

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integerlls = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, integerll<It, CharT, base>));
} // namespace basic
} // namespace rpc 

#endif // ifndef NUMERIC_PARSERS_HPP

