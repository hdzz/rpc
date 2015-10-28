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
#include "core/token_parsers.hpp"

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
    core::parser<It, std::basic_string<T>, R> natural_str =
        core::override_description
            (core::reducel
                (basic::digits<It, T, R>,
                [](T c, std::basic_string<T> & s)
                    { s.push_back (c); return s; },
                std::basic_string<T>()),
            "[natural]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> plus_natural_str =
        core::override_description
            (core::ignorel
                (core::token<It, T, R> ('+'),
                natural_str<It, T, R>),
            "[(+) natural]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> minus_natural_str = 
        core::override_description
            (core::reduce
                (core::sequence
                    (core::inject
                        (basic::character<It, T, R> ('-'),
                         std::basic_string<T> (1, '-')),
                    natural_str<It, T, R>)),
            "[(-) natural]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> decimal_str =
        core::override_description
            (core::reduce
                (core::sequence
                    (core::inject
                        (basic::character<It, T, R>('.'),
                         std::basic_string<T> (1, '.')),
                    natural_str<It, T, R>)),
            "[decimal]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    auto number_str =
        core::option
            (natural_str<It, T, R>,
            minus_natural_str<It, T, R>,
            plus_natural_str<It, T, R>);


     template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>  
    core::parser<It, std::basic_string<T>, R> exponent_str =
        core::override_description
            (core::reduce
                (core::sequence
                    (core::inject
                        (core::one_of<It, T, R>({'e','E'}),
                         std::basic_string<T>(1, 'e')),
                    number_str<It, T, R>)),
            "[exponent]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, std::basic_string<T>, R> floating_str =
        core::override_description
            (core::reduce
                (core::sequence
                    (number_str<It, T, R>,
                    core::optional (decimal_str<It, T, R>,
                                        std::basic_string<T> ()),
                    core::optional (exponent_str<It, T, R>,
                                        std::basic_string<T> ()))),
            "[(+/-) float]");
} // namespace detail

    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int>
    core::parser<It, IntT, R> todigit =
        core::override_description
            (core::lift
                (basic::digit<It, T, R>,
                [](T c) -> IntT { return static_cast<IntT> (c - '0'); }),
            "[digit]"); 


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int>
    core::parser<It, IntT, R> todigits = core::some (todigit<It, T, R, IntT>);


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int>
    core::parser<It, IntT, R> todigitm = core::many (todigit<It, T, R, IntT>);


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int>
    core::parser<It, IntT, R> towdigit =
        core::override_description
            (core::lift
                (basic::wdigit<It, T, R>,
                [](T c) -> IntT { return static_cast<IntT> (c - '0'); }),
            "[wide digit]");


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int>
    core::parser<It, IntT, R> towdigits = core::some (towdigit<It, T, R, IntT>);


    template <typename It,
        typename T = typename std::iterator_traits<It>::value_type,
        typename R = core::range<It>,
        typename IntT = unsigned int>
    core::parser<It, IntT, R> towdigitm = core::many (towdigit<It, T, R, IntT>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, unsigned long, R> natural = core::override_description
        (core::lift
            (detail::natural_str<It, T, R>,
            [](std::basic_string<T> const& s) -> unsigned long
                { return std::stoul (s, nullptr, base); }),
        "[nautral]");


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, unsigned long, R> naturals =
        core::some (natural<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, unsigned long, R> naturalm = 
        core::many (natural<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, unsigned long long, R> lnatural =
        core::override_description
            (core::lift
                (detail::natural_str<It, T, R>,
                [](std::basic_string<T> const& s) -> unsigned long long
                    { return std::stoull (s, nullptr, base); }),
            "[long natural]");


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, unsigned long long, R> lnaturals =
        core::some (lnatural<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, unsigned long long, R> lnaturalm =
        core::many (lnatural<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, int, R> integer = core::override_description
        (core::lift
            (detail::number_str<It, T, R>,
            [](std::basic_string<T> const& s) -> int
                { return std::stoi (s, nullptr, base); }),
        "[integer]");


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, int, R> integers = core::some (integer<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, int, R> integerm = core::many (integer<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, long, R> linteger = core::override_description
        (core::lift
            (detail::number_str<It, T, R>,
            [](std::basic_string<T> const& s) -> long
                { return std::stol (s, nullptr, base); }),
        "[long integer]");
 

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10> 
    core::parser<It, long, R> lintegers = core::some (linteger<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, long, R> lintegerm = core::many (linteger<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>   
    core::parser<It, long long, R> llinteger = core::override_description
        (core::lift
            (detail::number_str<It, T, R>,
            [](std::basic_string<T> const& s) -> long long
                { return std::stoll(s, nullptr, base); }),
        "[long long integer]");


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, long long, R> llintegers =
        core::some (llinteger<It, T, R, base>);


    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>, int base = 10>
    core::parser<It, long long, R> llintegerm =
        core::many (llinteger<It, T, R, base>);


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, float, R> floating = core::override_description
        (core::lift
            (detail::floating_str<It, T, R>,
            [](std::basic_string<T> const& s) -> float
                { return std::stof(s, nullptr); }),
        "[float]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, float, R> floatings = core::some (floating<It, T, R>);


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>   
    core::parser<It, float, R> floatingm = core::many (floating<It, T, R>);


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>   
    core::parser<It, double, R> lfloating = core::override_description
        (core::lift
            (detail::floating_str<It, T, R>,
            [](std::basic_string<T> const& s) -> double
                { return std::stod(s, nullptr); }),
        "[double]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, double, R> lfloatings = core::some (lfloating<It, T, R>);
 

    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>   
    core::parser<It, double, R> lfloatingm = core::many (lfloating<It, T, R>);
 

    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>   
    core::parser<It, long double, R> llfloating = core::override_description
        (core::lift
            (detail::floating_str<It, T, R>,
            [](std::basic_string<T> const& s) -> long double
                { return std::stold(s, nullptr); }),
        "[long double]");


    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, long double, R> llfloatings =
        core::some (llfloating<It, T, R>);
 

    template <typename It,
              typename T = typename std::iterator_traits<It>::value_type,
              typename R = core::range<It>>
    core::parser<It, long double, R> llfloatingm =
        core::many (llfloating<It, T, R>);
} // namespace basic
} // namespace rpc 

#endif // ifndef NUMERIC_PARSERS_HPP

