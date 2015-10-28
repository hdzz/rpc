//
// Constructing parsers from std::regex objects
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef REGEX_PARSERS_HPP
#define REGEX_PARSERS_HPP

#include <type_traits>
#include <iterator>
#include <list>
#include <regex>
#include <string>
#include <utility>

#include "core/range.hpp"
#include "core/parser.hpp"
#include "gsl/not_null.hpp"

namespace rpc
{
namespace basic
{
    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline core::parser<It, std::basic_string<T>, R> regexparser
        (std::basic_regex<T> const& rx, std::string const& pattern = "")
    {
        using A = typename core::parser
            <It, std::basic_string<T>, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;

        return core::parser<It, std::basic_string<T>, R>
        {
            .description =
                "[" + 
                (pattern.empty() ? "regex match" : (pattern + " match")) +
                "]",
            .parse =
            [=](AccT const acc)
            {
                std::match_results
                    <typename core::parser
                        <It, std::basic_string<T>, R>::range_type::iter_type>
                matches;
 
                auto start (core::torange (*acc));
                auto res = std::regex_search
                    (start.cbegin(), start.cend(), matches, rx,
                    std::regex_constants::match_continuous);
                if (res) {
                    acc->insert
                        (matches.str (0), start.tail (matches.length (0)));
                    return acc;
                } else {
                    acc->insert
                        (core::failure
                            {"expected [" +
                            (pattern.empty() ? "regex match"
                                             : (pattern + " match")) +
                            "]"},
                        start);
                    return acc;
                }
            }
        };
    }

    template <typename It,
             typename T = typename std::iterator_traits<It>::value_type,
             typename R = core::range<It>>
    inline core::parser<It, std::basic_string<T>, R> wregexparser
        (std::basic_regex<T> const& rx, std::string const& pattern = "")
    {
        using A = typename core::parser
            <It, std::basic_string<T>, R>::accumulator_type;
        using AccT = gsl::not_null_ptr<A>;
        
        return core::parser<It, std::basic_string<T>, R>
        {
            .description =
                "[" +
                (pattern.empty() ? "wregex match" : (pattern + " match")) +
                "]",
            .parse =
            [=](AccT const acc)
            {
                std::match_results
                    <typename core::parser
                        <It, std::basic_string<T>, R>::range_type::iter_type>
                matches;

                auto start (core::torange (*acc));
                auto res = std::regex_search
                    (start.cbegin(), start.cend(), matches, rx,
                    std::regex_constants::match_continuous);
                if (res) {
                    acc->insert
                        (matches.str (0), start.tail (matches.length (0)));
                    return acc;
                } else {
                    acc->insert
                        (core::failure
                            {"expected [" +
                            (pattern.empty() ? "wregex match"
                                             : (pattern + " match")) +
                            "]"},
                        start);
                    return acc;
                }
            }
        };
    }
} // namespace basic
} // namesapce rpc

#endif
