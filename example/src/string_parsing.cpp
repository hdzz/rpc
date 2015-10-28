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
#include "core/token_parsers.hpp"
#include "basic/numeric_parsers.hpp"
#include "basic/regex_parsers.hpp"

#include "funktional/include/foldable.hpp"
#include "funktional/include/mappable.hpp"
#include "funktional/include/utility/type_utils.hpp"

using namespace rpc;

template <typename T>
using iter_type = typename std::basic_string<T>::const_iterator;

auto rx_as    = basic::regexparser<iter_type<char>> (std::regex("a+"));
auto rx_as_bs = basic::regexparser<iter_type<char>> (std::regex("a+b+"));

template <typename T>
std::string print_results (std::deque<T> const& l)
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
static auto const parse_text_ints   = std::string ("13 45 99 803");
//static auto const parse_text_ints   = std::string ("-13 45 -99 +803");
static auto const parse_text_floats =
    std::string ("-2.3 3.14159 1 2e-2 -5.2E5");
static auto const parse_text_words  =
    std::string ("the quick brown fox jumped over the lazy dog");

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
        auto rxas_result = core::parse (rx_as, parse_text_chars);
        std::cout
            << '\t'
            << print_results (core::values (rxas_result))
            << std::endl;
        if (not rpc::core::parse_success (rxas_result))
            std::cout << "failed:\t"
                      << rpc::core::toresult_failure_message (rxas_result)
                      << std::endl;
    }

    std::cout << "rx_as_bs result:" << std::endl; 
    {
        auto rxasbs_result = core::parse (rx_as_bs, parse_text_chars);
        std::cout
            << '\t'
            << print_results (core::values (rxasbs_result))
            << std::endl;
        if (not rpc::core::parse_success (rxasbs_result))
            std::cout << "failed:\t"
                      << rpc::core::toresult_failure_message (rxasbs_result)
                      << std::endl;
    }

    std::cout << "whitespace result:" << std::endl;
    {
        auto spaces_result = core::parse
            (basic::spaces<iter_type<char>>, parse_text_ws);
        std::cout
            << '\t'
            << print_results
                (fnk::map (wsname, core::values (spaces_result)))
            << std::endl;
        if (not rpc::core::parse_success (spaces_result))
            std::cout << "failed:\t"
                      << rpc::core::toresult_failure_message (spaces_result)
                      << std::endl;
    }
/*
    std::cout << "digits (chars) result:" << std::endl;
    {
        auto parser = core::some
            (core::ignorer
                 (basic::digits<iter_type<char>>,
                 basic::spacem<iter_type<char>>));

        auto digits_result = core::parse
            (parser, parse_text_nats);

        std::cout
            << '\t'
            << print_results (core::values (digits_result))
            << std::endl;

        if (not rpc::core::parse_success (digits_result))
            std::cout << "failed:\t"
                      << rpc::core::toresult_failure_message (digits_result)
                      << std::endl;
    }

    std::cout << "digits (values) result:" << std::endl;
    {
        auto parser = core::some
            (core::ignorer
                 (basic::todigit<iter_type<char>>,
                 basic::spacem<iter_type<char>>));
        auto digits_result = core::parse
            (parser, parse_text_nats);

        std::cout
            << '\t'
            << print_results (core::values (digits_result))
            << std::endl;
   
        if (not rpc::core::parse_success (digits_result))
            std::cout << "failed:\t"
                      << rpc::core::toresult_failure_message (digits_result)
                      << std::endl;
    }
    
    std::cout << "naturals (strings) result:" << std::endl;
    {
        auto parser = core::some
            (core::ignorer
                 (basic::detail::natural_str<iter_type<char>>,
                 basic::spacem<iter_type<char>>));

        auto natstr_result = core::parse
            (parser, parse_text_nats);
        auto natstr_vals = core::values (natstr_result);

        std::cout << '\t' << print_results (natstr_vals) << std::endl;
        if (not rpc::core::parse_success (natstr_result))
            std::cout << "failed:\t"
                      << rpc::core::toresult_failure_message (natstr_result)
                      << std::endl;
    }
    
    std::cout << "naturals (values) result:" << std::endl;
    {
        auto parser = core::some
            (core::ignorer
                 (basic::natural<iter_type<char>>,
                 basic::spacem<iter_type<char>>));

        auto nats_result = core::parse
            (parser, parse_text_nats);
        auto nats_vals = core::values (nats_result);

        std::cout << '\t' << print_results (nats_vals) << std::endl;
        std::cout << "\tsum: " << fnk::fold (nats_vals) << std::endl;

        if (not rpc::core::parse_success (nats_result))
            std::cout << "failed:\t"
                      << rpc::core::toresult_failure_message (nats_result)
                      << std::endl;
    }

    std::cout << "integers (string) result:" << std::endl;
    {
        auto parser = core::some
            (core::ignorer
                 (basic::detail::number_str<iter_type<char>>,
                 basic::spacem<iter_type<char>>));
        
        auto intstr_result = core::parse
            (parser, parse_text_ints);
        auto intstr_vals = core::values (intstr_result);

        std::cout << '\t' << print_results (intstr_vals) << std::endl;
        if (not rpc::core::parse_success (intstr_result))
            std::cout << '\t'
                      << rpc::core::toresult_failure_message (intstr_result)
                      << std::endl;   
    }
   
    std::cout << "integers (values) result:" << std::endl;
    {
        auto parser = core::some
            (core::ignorer
                 (basic::integer<iter_type<char>>,
                 basic::spacem<iter_type<char>>));
        
        auto ints_result = core::parse
            (parser, parse_text_ints);
        auto ints_vals = core::values (ints_result);

        std::cout << '\t' << print_results (ints_vals) << std::endl;
        std::cout
            << "\taverage: "
            << static_cast<float> (fnk::fold (ints_vals)) /
               static_cast<float> (ints_vals.size())
            << std::endl;
        
        if (not rpc::core::parse_success (ints_result))
            std::cout << '\t'
                      << rpc::core::toresult_failure_message (ints_result)
                      << std::endl;   
    }
   
    std::cout << "floats (string) result:" << std::endl;
    {
        auto parser = core::some
            (core::ignorer
                 (basic::detail::floating_str<iter_type<char>>,
                 basic::spacem<iter_type<char>>));

        auto floatstr_result = core::parse
            (parser, parse_text_floats);
        auto floatstr_vals = core::values (floatstr_result);

        std::cout << '\t' << print_results (floatstr_vals) << std::endl;
        if (not rpc::core::parse_success (floatstr_result))
            std::cout << '\t'
                      << rpc::core::toresult_failure_message (floatstr_result)
                      << std::endl;
    }
   
    std::cout << "floats (values) result:" << std::endl;
    {
        auto parser = core::some
            (core::ignorer
                 (basic::floating<iter_type<char>>,
                 basic::spacem<iter_type<char>>));

        auto floats_result = core::parse
            (parser, parse_text_floats);
        auto floats_vals = core::values (floats_result);
        
        std::cout << '\t' << print_results (floats_vals) << std::endl;
        std::cout
            << "\tproduct: "
            << fnk::foldr
                ([](float x, float y) { return x * y; }, 1.0, floats_vals)
            << std::endl;
        
        if (not rpc::core::parse_success (floats_result))
            std::cout << '\t'
                      << rpc::core::toresult_failure_message (floats_result)
                      << std::endl;
    }
*/    
    std::cout << "words result:" << std::endl;
    {
        auto words_result = core::parse
            (basic::words<iter_type<char>>, parse_text_words);
        
        std::cout
            << '\t'
            << print_results (core::values (words_result))
            << std::endl;
        
        if (not rpc::core::parse_success (words_result))
            std::cout << '\t'
                      << rpc::core::toresult_failure_message (words_result)
                      << std::endl;
    }
/*
    std::cout << "failure looks like:" << std::endl;
    {
        auto fres = core::parse
            (basic::integers<iter_type<char>>, std::string(""));
        std::cout << '\t'
                  << rpc::core::toresult_failure_message (fres)
                  << std::endl;
    }
*/    
    return 0;
}

