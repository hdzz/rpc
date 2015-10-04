# rpc - (Real) Parser Combinators for C++14

This is an implementation of monadic parser combinators for C++14 using the
[`funktional`](https://github.com/daltonwoodard/funktional) C++14 library.
Much of this project's development was inspired and guided by [`Hutton, Graham, & Meijer, Erik. (1996). Monadic Parser
Combinators.`](http://www.cs.nott.ac.uk/~pszgmh/monparsing.pdf).

## Features

- Error reporting format designed to be parsed easily and pretty printed if desired.

## Notes

This project is still in development; there may be breaking changes made to core types for some time yet.
To that end, here is what still needs to be done and what has been completed already.

### To be completed
- [] Combinators:
    - [] `chainl`
    - [] `chainl1`;
    - [] `chainr`
    - [] `chainr1`
- [x] Better parser descriptions for constructing failure messages.
- [] Add infrastructure for creating ASTs from parse trees.
- [] Add parser and pretty printer for error messages.
- [] Documentation (maybe in wiki pages?).
- [] Unit testing for each basic parser and combinator listed below.

### Totally (or mostly) implemented
- Core types (`core/range`, `core/parser`):
    - `range`, the object representing a view of tokens to be parsed.
    - `parser`
        - The `parser` type is a `functor`, `applicative_functor`, `monad`, `monoid`, and `additive_monad`,
        and so all the standard operators for these types are supported.
- Basic Parsers (`basic/atom_parsers`):
    - `fail`
    - `unit`
    - `item`
    - `token`
    - `one_of`
    - `none_of`
    - `satisfy`
    - `in_range`
- Text parsers (various character types) (`basic/char_parsers`).
- Numeric parsers (the following and all variations thereof) (`basic/numeric_parsers`):
    - `todigit`
    - `natural`
    - `integer`
    - `floating`
- Regex parsers (construct a rpc::parser from std::regex object) (`basic/regex_parsers`).
- Combinators (`core/combinators`):
    - `bind`
    - `combine`
    - `sequence`; `sequence` and ignore left; `sequence` and ignore right
    - `option`
    - `optional`
    - `some`; `some` at least `n`
    - `many`; `many` up to `n`
    - `reduce`; `reduce` with a function `f`
    - `lift` to new value type; `lift` to a new value type with a function `f`
    - `liftreduce`; `liftreduce` with lift by a function `f`; `liftreduce` with lift by a function `f` and reduce
      with a function `g`

## Example - simple parsing of strings

```c++
#include <iostream>
#include <regex>
#include <string>
#include <utility>

#include "core/range.hpp"
#include "core/parser.hpp"
#include "core/combinators.hpp"
#include "basic/atom_parsers.hpp"
#include "basic/numeric_parsers.hpp"
#include "regex/regex_parsers.hpp"

using namespace rpc;

template <typename T>
using iter_type = typename std::basic_string<T>::const_iterator;

template <typename V, typename T = V>
using strparser = core::parser<iter_type<T>, V>;

auto as_bs = core::some (basic::one_of<iter_type<char>>({'a','b'}));

auto rx_as = basic::regexparser<iter_type<char>> (std::regex("a+"));
auto rx_as_bs = basic::regexparser<iter_type<char>> (std::regex("a+b+"));

static auto const parse_text_chars = std::string ("aabbb");
static auto const parse_text_ws    = std::string (" \n\t \r\v\f");
static auto const parse_text_nats  = std::string ("123 45 42 1001");
static auto const parse_text_ints  = std::string ("-13 45 -99 +803");
static auto const parse_text_floats = std::string ("-2.3 3.14159 1 2e-2 -5.2E5");
static auto const parse_text_words = std::string ("the quick brown fox jumped over the lazy dog");

int main (void)
{
    std::cout << "characters from as_bs:" << std::endl; 
    for (auto const& e : as_bs.parse (parse_text_chars))
        if (strparser<char>::is_result (e))
            std::cout << '\t' << strparser<char>::result_value (e) << std::endl;

    std::cout << "result from rx_as:" << std::endl; 
    for (auto const& e : rx_as.parse (parse_text_chars))
        if (strparser<std::string, char>::is_result (e))
            std::cout << '\t' << strparser<std::string, char>::result_value (e) << std::endl;

    std::cout << "result from rx_as_bs:" << std::endl; 
    for (auto const& e : rx_as_bs.parse (parse_text_chars))
        if (strparser<std::string, char>::is_result (e))
            std::cout << '\t' << strparser<std::string, char>::result_value (e) << std::endl;

    auto wsname = [] (char c)
    {
        if (c == ' ') return std::string("space");
        else if (c == '\n') return std::string("newline");
        else if (c == '\t') return std::string("tab");
        else if (c == '\v') return std::string("vertical tab");
        else if (c == '\r') return std::string("carriage return");
        else if (c == '\f') return std::string("form feed");
        else return std::string("not whitespace"); 
    };

    std::cout << "whitespace:" << std::endl;
    for (auto const& e : basic::spaces<iter_type<char>>.parse (parse_text_ws))
        if (strparser<char>::is_result (e))
            std::cout << '\t' << wsname (strparser<char>::result_value (e)) << std::endl;

    std::cout << "digits:" << std::endl;
    for (auto& e : basic::todigits<iter_type<char>>.parse (parse_text_nats))
        if (strparser<unsigned int, char>::is_result (e))
            std::cout << '\t' << strparser<unsigned int, char>::result_value(e) << std::endl;
   
    std::cout << "naturals:" << std::endl;
    for (auto const& e : basic::naturals<iter_type<char>>.parse (parse_text_nats))
        if (strparser<unsigned long, char>::is_result (e))
            std::cout << '\t' << strparser<unsigned long, char>::result_value(e) << std::endl;
    
    std::cout << "integers:" << std::endl;
    for (auto const& e : basic::integers<iter_type<char>>.parse (parse_text_ints))
        if (strparser<int, char>::is_result (e))
            std::cout << '\t' << strparser<int, char>::result_value(e) << std::endl;
 
    std::cout << "floats:" << std::endl;
    for (auto const& e : basic::floatings<iter_type<char>>.parse (parse_text_floats))
        if (strparser<float, char>::is_result (e))
            std::cout << '\t' << strparser<float, char>::result_value(e) << std::endl;
 
    std::cout << "words:" << std::endl;
    for (auto const& e : basic::words<iter_type<char>>.parse (parse_text_words))
        if (strparser<std::string, char>::is_result (e))
            std::cout << '\t' << strparser<std::string, char>::result_value(e) << std::endl;
 
    std::cout << "failure looks like:" << std::endl;

    auto f = basic::integers<iter_type<char>>.parse (std::string(""));
    std::cout << '\t' << strparser<int, char>::failure_message (f.front()) << std::endl;
   
    std::exit (0);
}
```

Produces the output:
```
characters from as_bs:
    a
    a
    b
    b
    b
result from rx_as:
    aa
result from rx_as_bs:
    aabbb
whitespace:
    space
    newline
    tab
    space
    carriage return
    vertical tab
    form feed
digits:
    1
    2
    3
    4
    5
    4
    2
    1
    0
    0
    1
naturals:
    123
    45
    42
    1001
integers:
    -13
    45
    -99
    803
floats:
    -2.3
    3.14159
    1
    0.02
    -520000
words:
    the
    quick
    brown
    fox
    jumped
    over
    the
    lazy
    dog
failure looks like:
    expected [(some) [[(many) [whitespace]] //then// [integer]]]
```

## LICENSE

Please see LICENSE.md included in this directory (`rpc/`)

