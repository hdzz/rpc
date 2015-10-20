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
    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    auto natural_str =
        core::override_description
            (core::reducer
                (digits<It, T, R>, 
                 [](std::basic_string<T>&s, T c) { s.push_back (c); return s; },
                 std::basic_string<T>()),
            "[natural]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    rpc::core::parser<It, std::basic_string<T>, R> plus_natural_str =
        rpc::core::override_description
            (rpc::core::ignorel
                (token<It, T, R> ('+'),
                natural_str<It, T, R>),
            "[(+) natural]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    rpc::core::parser<It, std::basic_string<T>, R> minus_natural_str = 
        rpc::core::override_description
            (rpc::core::reduce
                (rpc::core::sequence
                    (rpc::core::lift
                        (character<It, T, R>('-'),
                         [](T c) { return std::basic_string<T>(1, c); }),
                    natural_str<It, T, R>)),
            "[(-) natural]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    rpc::core::parser<It, std::basic_string<T>, R> decimal_str =
        rpc::core::override_description
            (rpc::core::reduce
                (rpc::core::sequence
                    (rpc::core::lift
                        (character<It, T, R>('.'),
                        [](T c) { return std::basic_string<T>(1, c); }),
                    natural_str<It, T, R>)),
            "[decimal]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    auto number_str =
        rpc::core::option
            (natural_str<It, T, R>,
            minus_natural_str<It, T, R>,
            plus_natural_str<It, T, R>);


     template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>  
    rpc::core::parser<It, std::basic_string<T>, R> exponent_str =
        rpc::core::override_description
            (rpc::core::reduce
                (rpc::core::sequence
                    (rpc::core::lift
                        (one_of<It, T, R>({'e','E'}),
                        [](T c) { return std::basic_string<T>(1, c); }),
                    number_str<It, T, R>)),
            "[exponent]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    rpc::core::parser<It, std::basic_string<T>, R> floating_str =
        rpc::core::override_description
            (rpc::core::reduce
                (rpc::core::sequence
                    (number_str<It, T, R>,
                    rpc::core::optional (decimal_str<It, T, R>, ""),
                    rpc::core::optional (exponent_str<It, T, R>, ""))),
            "[(+/-) float]");
} // namespace detail

    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int, int base = 0>
    auto todigit =
        rpc::core::override_description
            (rpc::core::lift
                (basic::digit<It, T, R>,
                [](T c) -> IntT { return c - '0'; }),
            "[digit]"); 


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int, int base = 0>
    auto todigits =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, todigit<It, T, R, IntT, base>));


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int, int base = 0>   
    auto todigitm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, todigit<It, T, R, IntT, base>));


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int, int base = 0>    
    auto towdigit =
        rpc::core::override_description
            (rpc::core::lift
                (basic::wdigit<It, T, R>,
                [](T c) -> IntT { return c - '0'; }),
            "[wide digit]");


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int, int base = 0>   
    auto towdigits =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, towdigit<It, T, R, IntT, base>));


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int, int base = 0>
    auto towdigitm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, towdigit<It, T, R, IntT, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto natural = rpc::core::override_description
        (rpc::core::lift
            (detail::natural_str<It, T, R>,
            [](std::basic_string<T> const& s) -> unsigned long
                { return std::stoul(s, nullptr, base); }),
        "[nautral]");


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto naturals =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, natural<It, T, R, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto naturalm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, natural<It, T, R, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto lnatural = rpc::core::override_description
        (rpc::core::lift
            (detail::natural_str<It, T, R>,
            [](std::basic_string<T> const& s) -> unsigned long long
                { return std::stoull(s,nullptr,base); }),
        "[long natural]");


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto lnaturals =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, lnatural<It, T, R, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto lnaturalm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, lnatural<It, T, R, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto integer = rpc::core::override_description
        (rpc::core::lift
            (detail::number_str<It, T, R>,
            [](std::basic_string<T> const& s) -> int
                { return std::stoi(s, nullptr, base); }),
        "[integer]");


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto integers =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, integer<It, T, R, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto integerm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, integer<It, T, R, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto linteger = rpc::core::override_description
        (rpc::core::lift
            (detail::number_str<It, T, R>,
            [](std::basic_string<T> const& s) -> long
                { return std::stol(s, nullptr, base); }),
        "[long integer]");
 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0> 
    auto lintegers =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, linteger<It, T, R, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto lintegerm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, linteger<It, T, R, base>));
 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>   
    auto llinteger = rpc::core::override_description
        (rpc::core::lift
            (detail::number_str<It, T, R>,
            [](std::basic_string<T> const& s) -> long long
                { return std::stoll(s, nullptr, base); }),
        "[long long integer]");


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto llintegers =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, llinteger<It, T, R, base>));


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 0>
    auto llintegerm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, llinteger<It, T, R, base>));


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    auto floating = rpc::core::override_description
        (rpc::core::lift
            (detail::floating_str<It, T, R>,
            [](std::basic_string<T> const& s) -> float
                { return std::stof(s, nullptr); }),
        "[float]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    auto floatings =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, floating<It, T, R>));


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>   
    auto floatingm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, floating<It, T, R>));


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>   
    auto lfloating = rpc::core::override_description
        (rpc::core::lift
            (detail::floating_str<It, T, R>,
            [](std::basic_string<T> const& s) -> double
                { return std::stod(s, nullptr); }),
        "[double]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    auto lfloatings =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, lfloating<It, T, R>));
 

    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>   
    auto lfloatingm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, lfloating<It, T, R>));
 

    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>   
    auto llfloating = rpc::core::override_description
        (rpc::core::lift
            (detail::floating_str<It, T, R>,
            [](std::basic_string<T> const& s) -> long double
                { return std::stold(s, nullptr); }),
        "[long double]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    auto llfloatings =
        rpc::core::some
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, llfloating<It, T, R>));
 

    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    auto llfloatingm =
        rpc::core::many
            (rpc::core::ignorel
                (rpc::basic::spacem<It, T, R>, llfloating<It, T, R>));
} // namespace basic
} // namespace rpc 

#endif // ifndef NUMERIC_PARSERS_HPP

