# funktional

funktional is a c++14 library for (mostly) pure functional programming.

## Some points on design philosophy

- Let RAII do it's work: With the exception of direct modification (insert, et cetera), structural
  transformations such as append, map, filter, fold, flatten, and so on should never modify data structures.

- Always capture by value when writing lambdas. This removes the possibility of confusion on a library user's part. If
  copies are too expensive to make, `std::ref` and `std::cref` can be used.

## TODO
- Implement recursive data type wrapper (`fix<F>` will not work without this).

## LICENSE

Please see LICENSE.md included in this directory (funktional/).

