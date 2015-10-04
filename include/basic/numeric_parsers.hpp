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
#include "text_parsers.hpp"

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
                return OT { rpc::core::failure{"expected [natural]"} };
        },
        .description = "[natural]"
    };

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>> plus_natural_str =
        rpc::core::override_description
            (rpc::core::ignorel (token<It, CharT> ('+'), natural_str<It, CharT>), "[(+) natural]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>> minus_natural_str = 
        rpc::core::override_description
            (rpc::core::reduce
                (rpc::core::sequence
                    (rpc::core::lift (character<It, CharT>('-'),
                                      [](CharT c) { return std::basic_string<CharT>(1, c); }),
                     natural_str<It, CharT>)),
            "[(-) natural]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>> decimal_str =
        rpc::core::override_description
            (rpc::core::reduce
                (rpc::core::sequence
                    (rpc::core::lift (character<It, CharT>('.'),
                                      [](CharT c) { return std::basic_string<CharT>(1, c); }),
                     natural_str<It, CharT>)),
            "[decimal]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto number_str = rpc::core::option (natural_str<It,CharT>, minus_natural_str<It,CharT>, plus_natural_str<It,CharT>);
   
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>> exponent_str =
        rpc::core::override_description
            (rpc::core::reduce
                (rpc::core::sequence
                    (rpc::core::lift (one_of<It, CharT>({'e','E'}),
                                      [](CharT c) { return std::basic_string<CharT>(1, c); }),
                     number_str<It, CharT>)),
            "[exponent]");
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    rpc::core::parser<It, std::basic_string<CharT>> floating_str =
        rpc::core::override_description
            (rpc::core::reduce
                (rpc::core::sequence
                    (number_str<It, CharT>,
                     rpc::core::optional (decimal_str<It, CharT>, std::string("")),
                     rpc::core::optional (exponent_str<It, CharT>, std::string("")))),
            "[(+/-) float]");

} // namespace detail
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto todigit = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, CharT>>::fmap
            ([](CharT c) -> IntT { return (c - '0'); }, basic::digit<It, CharT>),
        "[digit]"); 
 
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto todigits = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, todigit<It, CharT, IntT, base>));
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto todigitm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, todigit<It, CharT, IntT, base>));
     
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto towdigit = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, CharT>>::fmap
            ([](CharT c) -> IntT { return (c - '0'); }, basic::wdigit<It, CharT>),
        "[wide digit]"); 
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto towdigits = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, towdigit<It, CharT, IntT, base>)); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, typename IntT = unsigned int, int base = 0>
    auto towdigitm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, towdigit<It, CharT, IntT, base>)); 

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto natural = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
            ([](std::basic_string<CharT> const& s) -> unsigned long { return std::stoul(s, nullptr, base); },
            detail::natural_str<It, CharT>),
        "[nautral]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto naturals = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, natural<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto naturalm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, natural<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto lnatural = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
            ([](std::basic_string<CharT> const& s) -> unsigned long long { return std::stoull(s,nullptr,base); },
            detail::natural_str<It, CharT>),
        "[long natural]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto lnaturals = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, lnatural<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto lnaturalm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, lnatural<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integer = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
            ([](std::basic_string<CharT> const& s) -> int { return std::stoi(s, nullptr, base); },
            detail::number_str<It, CharT>),
        "[integer]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integers = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, integer<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto integerm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, integer<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto linteger = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
            ([](std::basic_string<CharT> const& s) -> long { return std::stol(s, nullptr, base); },
            detail::number_str<It, CharT>),
        "[long integer]");
  
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto lintegers = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, linteger<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto lintegerm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, linteger<It, CharT, base>));
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto llinteger = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
            ([](std::basic_string<CharT> const& s) -> long long { return std::stoll(s, nullptr, base); },
            detail::number_str<It, CharT>),
        "[long long integer]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto llintegers = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, llinteger<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type, int base = 0>
    auto llintegerm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, llinteger<It, CharT, base>));

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto floating = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
            ([](std::basic_string<CharT> const& s) -> float { return std::stof(s, nullptr); },
            detail::floating_str<It, CharT>),
        "[float]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto floatings = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, floating<It, CharT>));
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto floatingm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, floating<It, CharT>));
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto lfloating = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
            ([](std::basic_string<CharT> const& s) -> double { return std::stod(s, nullptr); },
            detail::floating_str<It, CharT>),
        "[double]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto lfloatings = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, lfloating<It, CharT>));
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto lfloatingm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, lfloating<It, CharT>));
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto llfloating = rpc::core::override_description
        (fnk::functor<rpc::core::parser<It, std::basic_string<CharT>>>::fmap
            ([](std::basic_string<CharT> const& s) -> long double { return std::stold(s, nullptr); },
            detail::floating_str<It, CharT>),
        "[long double]");

    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto llfloatings = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, llfloating<It, CharT>));
 
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    auto llfloatingm = rpc::core::many (rpc::core::ignorel (rpc::basic::spacem<It, CharT>, llfloating<It, CharT>));
} // namespace basic
} // namespace rpc 

#endif // ifndef NUMERIC_PARSERS_HPP

