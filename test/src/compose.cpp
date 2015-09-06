
#include <iostream>

#include "compose.hpp"

int main (void)
{
    auto h = funk::compose ([](int x) { return x*x; }, [](int x) { return x + 1; });
    std::cout
        << "[(x -> x^2) . (x -> x+1)] (2) = " 
        <<  h(2) << std::endl
        << "[(x -> x^2) . (x -> x+1)] (5) = "
        << h (5) << std::endl;
    return 0;
}

