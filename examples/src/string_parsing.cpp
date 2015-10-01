//
// Some basic examples of parsing
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#include <iostream>
#include <regex>
#include <string>
#include <type_traits>
#include <utility>

#include "core/range.hpp"
#include "core/parser.hpp"
#include "core/combinators.hpp"
#include "basic/atom_parsers.hpp"
#include "basic/numeric_parsers.hpp"
#include "regex/regex_parsers.hpp"

#include "funktional/include/foldable.hpp"
#include "funktional/include/zip.hpp"
#include "funktional/include/utility/bool_utils.hpp"

template <typename T>
using iter_type = typename std::basic_string<T>::const_iterator;

template <typename V, typename T = V>
using strparser = rpc::core::parser<iter_type<T>, V>;

auto as_bs = rpc::core::some (rpc::basic::one_of<iter_type<char>>({'a','b'}));

auto rx_as = rpc::regex::regexparser<iter_type<char>> (std::regex("a+"));
auto rx_as_bs = rpc::regex::regexparser<iter_type<char>> (std::regex("a+b+"));

static auto const parse_text_chars = std::string ("aabbb");
static auto const parse_text_ws    = std::string (" \n\t \r\v");
static auto const parse_text_nats  = std::string ("123 45 42 1001");
static auto const parse_text_ints  = std::string ("-13 45 -99 +803");
static auto const parse_text_words = std::string ("the quick brown fox jumped over the lazy dog");

int main (void)
{
    std::cout << "characters from as_bs:" << std::endl; 
    for (auto const& e : fnk::eval (as_bs.parse, parse_text_chars))
        if (strparser<char>::is_result (e))
            std::cout << '\t' << strparser<char>::result_value (e) << std::endl;

    std::cout << "result from rx_as:" << std::endl; 
    for (auto const& e : fnk::eval (rx_as.parse, parse_text_chars))
        if (strparser<std::string, char>::is_result (e))
            std::cout << '\t' << strparser<std::string, char>::result_value (e) << std::endl;

    std::cout << "result from rx_as_bs:" << std::endl; 
    for (auto const& e : fnk::eval (rx_as_bs.parse, parse_text_chars))
        if (strparser<std::string, char>::is_result (e))
            std::cout << '\t' << strparser<std::string, char>::result_value (e) << std::endl;

    auto wsname = [] (char c)
    {
        if (c == ' ') return std::string("space");
        else if (c == '\n') return std::string("newline");
        else if (c == '\t') return std::string("tab");
        else if (c == '\v') return std::string("vertical tab");
        else if (c == '\r') return std::string("carriage return");
        else return std::string("not whitespace"); 
    };

    std::cout << "whitespace:" << std::endl; 
    for (auto const& e : fnk::eval (rpc::basic::spaces<iter_type<char>>.parse, parse_text_ws))
        if (strparser<char>::is_result (e))
            std::cout << '\t' << fnk::eval (wsname, strparser<char>::result_value (e)) << std::endl;

    std::cout << "digits:" << std::endl;
    for (auto& e : fnk::eval (rpc::basic::todigits<iter_type<char>>.parse, parse_text_nats))
        if (strparser<unsigned int, char>::is_result (e))
            std::cout << '\t' << strparser<unsigned int, char>::result_value(e) << std::endl;
   
    std::cout << "naturals:" << std::endl;
    for (auto const& e : fnk::eval (rpc::basic::naturals<iter_type<char>>.parse, parse_text_nats))
        if (strparser<unsigned long, char>::is_result (e))
            std::cout << '\t' << strparser<unsigned long, char>::result_value(e) << std::endl;
    
    std::cout << "integers:" << std::endl;
    for (auto const& e : fnk::eval (rpc::basic::integers<iter_type<char>>.parse, parse_text_ints))
        if (strparser<int, char>::is_result (e))
            std::cout << '\t' << strparser<int, char>::result_value(e) << std::endl;
 
    std::cout << "words:" << std::endl;
    for (auto const& e : fnk::eval (rpc::basic::words<iter_type<char>>.parse, parse_text_words))
        if (strparser<std::string, char>::is_result (e))
            std::cout << '\t' << strparser<std::string, char>::result_value(e) << std::endl;
    
    std::exit (0);
}

