//
// A small example of recursive algebraic data type usage
//

#include <iostream>
#include <list>

#include "filterable.hpp"
#include "mappable.hpp"
#include "algebraic.hpp"

struct expr;

struct lit
{
    int val;
};

template <typename T>
struct add
{
    T l;
    T r;
};

template <typename T>
struct mul
{
    T l;
    T r;
};

struct expr
{
    fnk::adt<lit, fnk::recursive<add<expr>>, fnk::recursive<mul<expr>>> e_;
};

decltype(auto) eval (lit const& e) { return e.val; }

template <typename T>
decltype(auto) eval (add<T> const& e) { return eval (e.l) + eval (e.r); }

template <typename T>
decltype(auto) eval (mul<T> const& e) { return eval (e.l) * eval (e.r); }

decltype(auto) eval (expr const& ex)
{
    switch (ex.e_.type_index())
    {
        case 0:
            return eval (ex.e_.template value<lit>());
            break;
        case 1:
            return eval (ex.e_.template value<add<expr>>());
        case 2:
        default:
            return eval (ex.e_.template value<mul<expr>>());
    }
}

int main (void)
{
    expr ex = { mul<expr> { expr {lit{5}}, {add<expr> {{lit{1}} , {lit{2}}}} }};
    std::cout << "5 * (1 + 2) = " << eval (ex) << std::endl;
    return 0;
}

