# rpc - (Real) Parser Combinators for C++14

This is an implementation of monadic parser combinators for C++14 using the
[`funktional`](https://github.com/daltonwoodard/funktional) C++14 library.
Much of this project's development was inspired and guided by [`Hutton, Graham, & Meijer, Erik. (1996). Monadic Parser
Combinators.`](http://www.cs.nott.ac.uk/~pszgmh/monparsing.pdf).

## Notes

This project is still in development; there may be breaking changes made to core types for some time yet.
To that end, here is what still needs to be done and what has been completed already.

### To be completed (check indicates work is in progress)
- [ ] Combinators:
    - [ ] `chainl`
    - [ ] `chainl1`;
    - [ ] `chainr`
    - [ ] `chainr1`
- [x] Convert to continuation passing style for combinators to remove incidental structure in parsing (this will
  drastically alter much of the code, so it's currently being developed on branch `continuations`).
- [x] Better parser descriptions for constructing failure messages.
- [ ] Add infrastructure for error recovery.
- [x] Add infrastructure for creating ASTs from parse trees.
- [ ] Generalize output container type from std::list (using std::vector may be more desirable for high-performance cases)
- [ ] Use allocator-awareness where applicable.
- [ ] Add parser and pretty printer for error messages.
- [ ] Documentation and library reference (maybe create wiki pages?).
- [ ] Unit testing for each basic parser and combinator listed below.

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
    - `inject` a value replacing a successful parse result

## Example - simple parsing of strings

```c++
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
        std::cout
            << '\t'
            << print_list (strparser<std::string, char>::values (rxas_result))
            << std::endl;
    }
    
    std::cout << "rx_as_bs result:" << std::endl; 
    {
        auto rxasbs_result = rx_as_bs.parse (parse_text_chars);
        std::cout
            << '\t'
            << print_list (strparser<std::string, char>::values (rxasbs_result))
            << std::endl;
    }
    
    std::cout << "whitespace result:" << std::endl;
    {
        auto spaces_result = basic::spaces<iter_type<char>>.parse (parse_text_ws);
        std::cout
            << '\t'
            << print_list (fnk::map (wsname, strparser<char, char>::values (spaces_result)))
            << std::endl;
    }
    
    std::cout << "digits result:" << std::endl;
    {
        auto digits_result = basic::todigits<iter_type<char>>.parse (parse_text_nats);
        std::cout
            << '\t'
            << print_list (strparser<unsigned int, char>::values (digits_result))
            << std::endl;
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
        std::cout
            << "\taverage: "
            << static_cast<float>(fnk::fold (ints_vals)) / static_cast<float>(ints_vals.size())
            << std::endl;
    }
    
    std::cout << "floats result:" << std::endl;
    {
        auto floats_result = basic::floatings<iter_type<char>>.parse (parse_text_floats);
        auto floats_vals   = strparser<float, char>::values (floats_result);
        std::cout << '\t' << print_list (floats_vals) << std::endl;
        std::cout
            << "\tproduct: "
            << fnk::foldr ([](float x, float y) { return x * y; }, 1.0, floats_vals)
            << std::endl;
    }

    std::cout << "words result:" << std::endl;
    {
        auto words_result = basic::words<iter_type<char>>.parse (parse_text_words);
        std::cout
            << '\t'
            << print_list (strparser<std::string, char>::values (words_result))
            << std::endl;
    }

    std::cout << "failure looks like:" << std::endl;
    {
        auto f = basic::integers<iter_type<char>>.parse (std::string(""));
        std::cout << '\t' << strparser<int, char>::failure_message (f.front()) << std::endl;
    }

    std::exit (0);
}
```

Produces the output:
```
$ ./examples/build/string_parsing.out 
rx_as result:
    [aa]
rx_as_bs result:
    [aabbb]
whitespace result:
    [space, newline, tab, space, carriage return, vertical tab, form feed]
digits result:
    [1, 2, 3, 4, 5, 4, 2, 1, 0, 0, 1]
naturals result:
    [123, 45, 42, 1001]
    sum: 1211
integers result:
    [-13, 45, -99, 803]
    average: 184
floats result:
    [-2.3, 3.14159, 1, 0.02, -520000]
    product: 75146.8
words result:
    [the, quick, brown, fox, jumped, over, the, lazy, dog]
failure looks like:
    expected [(some) [[(many) [whitespace]] //then// [integer]]]
```

