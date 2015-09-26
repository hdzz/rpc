//
// A tiny parser library using `funktional`
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#include <iostream>
#include <string>

#include "parser.hpp"

template <char c>
fnk::parser<char> character = fnk::satisfy([](char a) { return c == a; });

fnk::parser<char> is_digit = fnk::satisfy([](char a) { return '0' <= a && a <= '9'; });

fnk::parser<int, char> natural = fnk::functor<fnk::parser<char>>::fmap ([](char c) -> int { return c - '0'; }, fnk::some (is_digit));

static auto const parse_text_chars = std::list<char> { 'a', 'a', 'b', 'b', 'b' };
static auto const parse_text_nats  = std::list<char> { '1', '2', '3', '4', '5' };

int main (void)
{
    auto as_bs = fnk::sequence (fnk::some(character<'a'>), fnk::some(character<'b'>));
    
    for (auto& e : as_bs.parse(parse_text_chars))
        std::cout << e.first << std::endl;

    for (auto& e : natural.parse(parse_text_nats))
        std::cout << e.first << std::endl;

    return 0;
}

