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

#include "parser.hpp"
#include "zip.hpp"

#include "utility/bool_utils.hpp"

template <char c>
fnk::parser<char> character = fnk::satisfy ([](char a) { return c == a; });

template <char ... cs>
fnk::parser<char> one_of = fnk::satisfy ([](char a) { return fnk::utility::any_true((a == cs)...); });

fnk::parser<char> whitespace = one_of<' ','\t','\n','\r'>;

fnk::parser<char> is_digit = fnk::satisfy([](char a) { return '0' <= a && a <= '9'; });

fnk::parser<int, char> digit = fnk::functor<fnk::parser<char>>::fmap ([](char c) -> int { return c - '0'; }, fnk::some (is_digit));

fnk::parser<int, char> natural =
{
    .parse = [] (std::list<char> const& s)
    {
        auto r1 = fnk::eval (digit.parse, s);
        int p = r1.size() - 1;
        int r2 = 0;
        for (auto& d : r1)
            r2 += d.first * std::pow (10, p--);
        return std::list<std::pair<int,std::list<char>>> { std::make_pair(r2, r1.back().second) };
    }
};

fnk::parser<int, char> naturals = fnk::some (fnk::ignorel(fnk::many(whitespace), natural));

static auto const parse_text_chars = std::list<char> { 'a', 'a', 'b', 'b', 'b' };
static auto const parse_text_nats  = std::list<char> { ' ', '1', '2', '3', '4', '5', ' ', '4', '2' };

int main (void)
{
    auto as_bs = fnk::sequence (fnk::some(character<'a'>), fnk::some(character<'b'>));
   
    std::cout << "characters:" << std::endl; 
    for (auto& e : fnk::eval (as_bs.parse, parse_text_chars))
        std::cout << '\t' << e.first << std::endl;
/*
    std::cout << "digits" << std::endl;
    for (auto& e : fnk::eval (digit.parse, parse_text_nats))
        std::cout << '\t' << e.first << std::endl;
*/
    std::cout << "numbers" << std::endl;
    for (auto& e : fnk::eval (naturals.parse, parse_text_nats))
        std::cout << '\t' << e.first << std::endl;

    return 0;
}

