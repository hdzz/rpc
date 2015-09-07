//
// Algebraic (variant) data types in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ALGEBRAIC_TYPES_HPP
#define ALGEBRAIC_TYPES_HPP

//
// Algebraic types are discriminated unions of types T1, T2, ..., TN,
// initialized once with a single fixed type Ti, which is invariant through
// the lifetime of the object. Moreover, such a type supports a never-empty guarantee;
// that is, expressions of the form
//      fnk::adt<T1,T2,...,TN> a;
// are not supported. Rather, they must be initialized at declaration with an expression
// of type Ti, implicitly convertible to one of T1, ..., TN.
// 
// Note: this is NOT the same as a boost::variant type.
//

namespace fnk
{
     
} // namespace fnk

#endif // ifndef ALGEBRAIC_TYPES_HPP

