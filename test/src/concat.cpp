
#include <iostream>
#include <list>

#include "concat.hpp"

int main (void)
{
    std::list<int const> l1 { 1, 2, 3, 4, 5, 6 };
    std::list<std::string const> l2 { "test", " ", "concat", "."};
    std::list<std::list<std::string const> const> l3 = {{"test", " ", "concat"}, {" "}, {"composed", " ", "twice", "."}};

    std::cout << funk::concat (l1) << std::endl;
    std::cout << funk::concat (l2) << std::endl;
    std::cout << funk::concat (funk::concat (l3)) << std::endl;
    return 0;
}

