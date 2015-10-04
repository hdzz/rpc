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
#include <sstream>
#include <utility>

#include "core/range.hpp"
#include "core/parser.hpp"
#include "core/combinators.hpp"
#include "basic/atom_parsers.hpp"
#include "basic/numeric_parsers.hpp"
#include "basic/regex_parsers.hpp"

#include "funktional/include/foldable.hpp"
#include "funktional/include/mappable.hpp"
#include "funktional/include/utility/type_utils.hpp"

using namespace rpc;

template <typename T>
using iter_type = typename std::basic_string<T>::const_iterator;

template <typename V, typename T = V>
using strparser = core::parser<iter_type<T>, V>;

auto rx_as = basic::regexparser<iter_type<char>> (std::regex("a+"));
auto rx_as_bs = basic::regexparser<iter_type<char>> (std::regex("a+b+"));

template <typename T>
decltype(auto) print_list (std::list<T> const& l)
{
    std::stringstream out;
    if (not l.empty()) {
        out << "[";
        for (auto it = l.cbegin(); it != std::prev(l.cend()); ++it)
            out << fnk::utility::to_string<T> (*it) + ", ";
        out << fnk::utility::to_string<T>(l.back()) + "]";
    } else
        out << "[]";
    return out.str();
}

static auto const parse_text_chars  = std::string ("aabbb");
static auto const parse_text_ws     = std::string (" \n\t \r\v\f");
static auto const parse_text_nats   = std::string ("123 45 42 1001");
static auto const parse_text_ints   = std::string ("-13 45 -99 +803");
static auto const parse_text_floats = std::string ("-2.3 3.14159 1 2e-2 -5.2E5");
static auto const parse_text_words  = std::string ("the quick brown fox jumped over the lazy dog");
static auto const wsname = [] (char c)
{
    if (c == ' ') return std::string("space");
    else if (c == '\n') return std::string("newline");
    else if (c == '\t') return std::string("tab");
    else if (c == '\v') return std::string("vertical tab");
    else if (c == '\r') return std::string("carriage return");
    else if (c == '\f') return std::string("form feed");
    else return std::string("not whitespace"); 
};

int main (void)
{
    std::cout << "rx_as result:" << std::endl; 
    {
        auto rxas_result = rx_as.parse (parse_text_chars);
        std::cout << '\t' << print_list (strparser<std::string, char>::values (rxas_result)) << std::endl;
    }
    
    std::cout << "rx_as_bs result:" << std::endl; 
    {
        auto rxasbs_result = rx_as_bs.parse (parse_text_chars);
        std::cout << '\t' << print_list (strparser<std::string, char>::values (rxasbs_result)) << std::endl;
    }
    
    std::cout << "whitespace result:" << std::endl;
    {
        auto spaces_result = basic::spaces<iter_type<char>>.parse (parse_text_ws);
        std::cout << '\t' << print_list (fnk::map (wsname, strparser<char, char>::values (spaces_result))) << std::endl;
    }
    
    std::cout << "digits result:" << std::endl;
    {
        auto digits_result = basic::todigits<iter_type<char>>.parse (parse_text_nats);
        std::cout << '\t' << print_list (strparser<unsigned int, char>::values (digits_result)) << std::endl;
    }
    
    std::cout << "naturals result:" << std::endl;
    {
        auto nats_result = basic::naturals<iter_type<char>>.parse (parse_text_nats);
        auto nats_vals   = strparser<unsigned long, char>::values (nats_result);
        std::cout << '\t' << print_list (nats_vals) << std::endl;
        std::cout << "\tsum: " << fnk::fold (nats_vals) << std::endl;
    }
    
    std::cout << "integers result:" << std::endl;
    {
        auto ints_result = basic::integers<iter_type<char>>.parse (parse_text_ints);
        auto ints_vals   = strparser<int, char>::values (ints_result);
        std::cout << '\t' << print_list (ints_vals) << std::endl;
        std::cout << "\tsum: " << fnk::fold (ints_vals) << std::endl;
    }
    
    std::cout << "floats result:" << std::endl;
    {
        auto floats_result = basic::floatings<iter_type<char>>.parse (parse_text_floats);
        auto floats_vals   = strparser<float, char>::values (floats_result);
        std::cout << '\t' << print_list (floats_vals) << std::endl;
        std::cout << "\tproduct: " << fnk::foldr ([](float x, float y) { return x * y; }, 1.0, floats_vals) << std::endl;
    }

    std::cout << "words result:" << std::endl;
    {
        auto words_result = basic::words<iter_type<char>>.parse (parse_text_words);
        std::cout << '\t' << print_list (strparser<std::string, char>::values (words_result)) << std::endl;
    }

    std::cout << "failure looks like:" << std::endl;
    {
        auto f = basic::integers<iter_type<char>>.parse (std::string(""));
        std::cout << '\t' << strparser<int, char>::failure_message (f.front()) << std::endl;
    }

    std::exit (0);
}

