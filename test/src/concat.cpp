
#include <iostream>
#include <list>

#include "concat.hpp"

int main (void)
{
    std::list<int const> l1 { 1, 2, 3, 4, 5, 6 };
    std::list<std::string const> l2 { "test", " ", "concat", "."};
    std::list<std::list<std::string const> const> l3 = {{"test", " ", "concat"}, {" "}, {"composed", " ", "twice", "."}};
    std::list<std::list<std::list<std::string const> const> const> l4 =
        {{{"test"}, {" "}, {"concat"}, {" "}},{{"composed"}, {" "}},{{"thrice"}, {"."}}};

    std::cout << fnk::concat (l1) << std::endl;
    std::cout << fnk::concat (l2) << std::endl;
    std::cout << fnk::concat (fnk::concat (l3)) << std::endl;
    std::cout << fnk::concat (fnk::concat (fnk::concat (l4))) << std::endl;
    return 0;
}

