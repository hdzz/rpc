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

namespace rpc
{
namespace basic
{
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    inline constexpr decltype(auto) regexparser (std::basic_regex<CharT> const& rx)
    {
        using OT = std::list<typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::result_type>;
        return rpc::core::parser<It, std::basic_string<CharT>, CharT>
        {
            .parse = [=](typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::range_type const& r)
            {
                std::match_results<typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::range_type::iter_type> matches;
                if (std::regex_search (r.getstart(), r.getend(), matches, rx, std::regex_constants::match_continuous)) {
                    return OT
                        {
                            std::make_pair (matches.str (0), r.tail (matches.length(0)))
                        };
                } else
                    return OT { core::failure {"could not match to regex"} };
            }
        };
    }
    
    template <typename It, typename CharT = typename std::iterator_traits<It>::value_type>
    inline constexpr decltype(auto) wregexparser (std::basic_regex<CharT> const& rx)
    {
        using OT = std::list<typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::result_type>;
        return rpc::core::parser<It, std::basic_string<CharT>, CharT>
        {
            .parse = [=](typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::range_type const& r)
            {
                std::match_results<typename rpc::core::parser<It, std::basic_string<CharT>, CharT>::range_type::iter_type> matches;
                if (std::regex_search (r.getstart(), r.getend(), matches, rx, std::regex_constants::match_continuous)) {
                    return OT
                        {
                            std::make_pair (matches.str (0), r.tail (matches.length(0)))
                        };
                } else
                    return OT { core::failure {"could not match to regex"} };
            }
        };
    }
} // namespace basic
} // namesapce rpc

#endif

