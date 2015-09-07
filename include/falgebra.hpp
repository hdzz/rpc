//
// F-Algebras in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// LICENSE: Please see LICENSE.md
//

#ifndef FALGEBRA_HPP
#define FALGEBRA_HPP

#include <type_traits>

#include "functor.hpp"
#include "type_support/fix_type.hpp"

namespace fnk
{
    //
    // generalized catamorphisms
    //
    template <typename A, template <typename> class F,
        typename = std::enable_if<fnk::functor<F<fnk::fix<F>>>::is_functor_instance::value>>
    constexpr decltype(auto) cata (A && alg, fnk::fix<F> && o)
    {
        return fnk::eval
            (alg, 
             fnk::functor<F<fnk::fix<F>>>::fmap
                ([=](auto&& o_) { return fnk::eval (cata<A,F>, alg, o_); }, fnk::unfx (std::forward<fnk::fix<F>>(o))));
    }
    
    template <typename A, template <typename> class F,
        typename = std::enable_if<fnk::functor<F<fnk::fix<F>>>::is_functor_instance::value>>
    constexpr decltype(auto) cata (A && alg, fnk::fix<F> const& o)
    {
        return fnk::eval
            (alg, 
             fnk::functor<F<fnk::fix<F>>>::fmap
                ([=](auto&& o_) { return fnk::eval (cata<A,F>, alg, o_); }, fnk::unfx (o)));
    }
} // namespace fnk

#endif // ifndef FALGEBRA_HPP

