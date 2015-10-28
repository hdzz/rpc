# rpc - (Real) Parser Combinators for C++14

This is an implementation of monadic parser combinators in meant to be used in
continuation passing style for C++14. It is developed using the
[`funktional`](https://github.com/daltonwoodard/funktional) C++14 library.
Much of this project's development was inspired and guided by the following
papers:

- [Hutton, Graham, & Meijer, Erik. (1996). Monadic Parser Combinators.]
(http://www.cs.nott.ac.uk/~pszgmh/monparsing.pdf)
- [Koopman, Pieter & Plasmeijer, Rinus. (1998). Efficient Combinator Parsers.]
(http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.36.2596)
- [Scott, Elizabeth & Johnstone, Adrian. (2009). GLL Parsing.
Preliminary Proceedings of the Ninth Workshop on
Language Descriptions Tools and Applications LDTA 2009.]
(http://ldta.info/2009/ldta2009proceedings.pdf)

## Some Points on Design

- Parsers should only ever produce a *single* value; the use of lifts and
reductions should be used to achieve this.
- When possible, parsers should be composed using the `branch` methods for
continuation passing style.

## Why is Continuation Passing Style encouraged with the family of branch \
methods? (tldr; incidental structure is bad)

Using CPS in a parser combinator framework, while initially more difficult with
respect to syntactic constructs, provides an extremely convenient means of
avoiding incidental data structures in code. The benefit of this is mainly that
it becomes unnecessary to construct temporary parse result lists, which saves
time and space, and we can instead merely append to an accumulator that the
combinators seamlessly thread through the call chain for us. This may also make
it easier for compilers to optimize parser code, as the vast majority of the
source is reduced to function calls, rather than building up temporary data
structures to be returned and modified explicitly by the programmer.

From an efficiency standpoint, there is good evidence that writing parser
combinator frameworks in continuation passing style can produce significantly
faster and more space efficient code; see [Efficient Combinator Parsers]
(http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.36.2596).

## Why don't the parsers use the "list of success" method?

Because incidental structure is bad, and returning a list of successes
requires writing unnecessary code to compose parsers. Moreover, it makes
direct composition of parsers non-trivial to accomplish.

## Why can't my parsers return a list of values?

They can, just so long as that is the declared parser value; otherwise, see
above.

## Why is std::deque the default backing container for the accumulator type?

Because [linked lists are bad]
(https://www.youtube.com/watch?v=YQs6IC-vgmo&ab_channel=AlessandroStamatto).

In all seriousness, [`std::deque`]
(http://baptiste-wicht.com/posts/2012/12/cpp-benchmark-vector-list-deque.html)
gives the right mix of performance characteristics needed for efficient parsing.

## Notes

This project is still in development; there may be breaking changes made to core
types for some time yet. To that end, here is what still needs to be done and 
what has been completed already.

### To be completed (check indicates work is in progress)
- [ ] Combinators:
    - [ ] `chainl`
    - [ ] `chainl1`;
    - [ ] `chainr`
    - [ ] `chainr1`
- [x] Better parser descriptions for constructing failure messages.
- [ ] Add infrastructure for error recovery.
- [x] Add infrastructure for creating ASTs from parse trees.
- [ ] Use allocator-awareness where applicable.
- [ ] Add parser and pretty printer for error messages.
- [ ] Documentation and library reference (maybe create wiki pages?).
- [ ] Unit testing for each basic parser and combinator listed below.

### Totally (or mostly) implemented
- Core types (`core/range`, `core/parser`):
    - `range`, the object representing a view of tokens to be parsed.
    - `parser`
        - The `parser` type is a `functor`, `applicative_functor`, `monad`,
        `monoid`, and `additive_monad`, and so all the standard operators for
        these types are supported.
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
- Numeric parsers (the following and all variations thereof)
(`basic/numeric_parsers`):
    - `todigit`
    - `natural`
    - `integer`
    - `floating`
- Regex parsers (construct a rpc::parser from std::regex object)
(`basic/regex_parsers`).
- Combinators (`core/combinators`):
    - `bind`
    - `combine`
    - `sequence`; `sequence` and ignore left; `sequence` and ignore right
    - `option`
    - `optional`
    - `some`; `some` at least `n`
    - `many`; `many` up to `n`
    - `reduce`; `reducel` (foldl) with a function `f`; `reducer` (foldr) with a
    function `f`.
    - `lift` to new value type; `lift` to a new value type with a function `f`
    - `liftreduce`; `liftreduce` with lift by a function `f`; `liftreducel` with
    lift by a function `f` and reduce with a function `g`; `liftreducer` with
    lift by a function `f` and reduce with a function `g`.
    - `inject` a value replacing a successful parse result

