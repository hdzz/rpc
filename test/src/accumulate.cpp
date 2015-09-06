
#include <iostream>
#include <string>
#include <list>

#include "accumulate.hpp"

int main (void)
{
    std::list<int const> l {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto f = [] (int const x) { return x*x; };
    std::cout << "sum: " << funk::accumulate (l) << std::endl;
    std::cout << "sum of squares: " << funk::accumulate (l, f) << std::endl;
    return 0;
}

