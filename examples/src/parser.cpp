//
// A tiny parser library using `funktional`
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#include <iostream>
#include <string>
#include <utility>

#include "foldable.hpp"
#include "parser.hpp"
#include "zip.hpp"

#include "utility/bool_utils.hpp"

template <char c>
fnk::parse::parser<char> character = fnk::parse::satisfy ([](char a) { return c == a; });

template <char ... cs>
fnk::parse::parser<char> one_of = fnk::parse::satisfy ([](char a) { return fnk::utility::any_true((a == cs)...); });

fnk::parse::parser<char> whitespace = one_of<' ','\t','\n','\r'>;

fnk::parse::parser<char> is_digit = fnk::parse::satisfy([](char a) { return '0' <= a && a <= '9'; });

fnk::parse::parser<int, char> digit =
    fnk::functor<fnk::parse::parser<char>>::fmap ([](char c) -> int { return c - '0'; }, fnk::parse::some (is_digit));

fnk::parse::parser<int, char> natural =
{
    .parse = [] (std::list<char> const& s)
    {
        auto r = fnk::eval (digit.parse, s);
        if (decltype(digit)::is_value(r.front().first)) {
            auto vs = decltype(digit)::values(r);
            auto n = fnk::foldl ([p = vs.size()-1](int a, int b) mutable { return a + (b * std::pow(10, p--)); }, 0, vs);
            return std::list<std::pair<decltype(digit)::return_type,std::list<char>>> { std::make_pair(n, r.back().second) };
        } else
            return std::list<std::pair<decltype(digit)::return_type,std::list<char>>>
                { std::make_pair(fnk::parse::detail::failure{.msg = std::string("expected natural")}, r.front().second) };
    }
};

static auto const parse_text_chars = std::list<char> { 'a', 'a', 'b', 'b', 'b' };
static auto const parse_text_ws    = std::list<char> { ' ', '\n', '\t', ' ', '\r' };
static auto const parse_text_nats  = std::list<char> { ' ', '1', '2', '3', '4', '5', ' ', '4', '2' };

int main (void)
{
    // these are similar parsers that should both match everyting in
    // `parse_text_chars`
    auto as_bs1 = fnk::parse::sequence (fnk::parse::some(character<'a'>), fnk::parse::some(character<'b'>));
    auto as_bs2 = fnk::parse::some (one_of<'a','b'>);

    auto whitespaces = fnk::parse::many (whitespace);

    auto digits = fnk::parse::some (fnk::parse::ignorel(whitespaces, digit));
    auto naturals = fnk::parse::some (fnk::parse::ignorel(whitespaces, natural));

    std::cout << "characters from as_bs1:" << std::endl; 
    for (auto const& e : fnk::eval (as_bs1.parse, parse_text_chars))
        if (fnk::parse::is_value (e))
            std::cout << '\t' << fnk::parse::value (e) << std::endl;
    
    std::cout << "characters from as_bs2:" << std::endl; 
    for (auto const& e : fnk::eval (as_bs2.parse, parse_text_chars))
        if (fnk::parse::is_value (e))
            std::cout << '\t' << fnk::parse::value (e) << std::endl;

    auto wsname = [] (char c)
    {
        if (c == ' ') return std::string("space");
        else if (c == '\n') return std::string("newline");
        else if (c == '\t') return std::string("tab");
        else if (c == '\r') return std::string("carriage return");
        else return std::string("not whitespace"); 
    };

    std::cout << "whitespace:" << std::endl; 
    for (auto const& e : fnk::eval (whitespaces.parse, parse_text_ws))
        if (fnk::parse::is_value (e))
            std::cout << '\t' << fnk::eval (wsname, fnk::parse::value (e)) << std::endl;

    std::cout << "digits" << std::endl;
    for (auto& e : fnk::eval (digits.parse, parse_text_nats))
        if (fnk::parse::is_value (e))
            std::cout << '\t' << fnk::parse::value(e) << std::endl;
    
    std::cout << "numbers" << std::endl;
    for (auto const& e : fnk::eval (naturals.parse, parse_text_nats))
        if (fnk::parse::is_value (e))
            std::cout << '\t' << fnk::parse::value(e) << std::endl;
    return 0;
}

