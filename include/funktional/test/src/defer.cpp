
#include <iostream>

#include "defer.hpp"

struct foo
{
    std::string bar;
};

void function (int const& x)
{
    std::cout << "got a number: " << x << std::endl;
}

int main (void)
{
    int arg1 = 1;
    int arg2 = 2;
    auto deferred1 = fnk::defer (function, arg1);
    std::cout << "just deferred 'function' with argument: " << arg1 << "." << std::endl;
    std::cout << "when called not deferred with argument: " << arg2 << "." << std::endl;
    function (arg2);
    std::cout << "when called deferred:" << std::endl;
    deferred1 ();
    
    // when defered with objects that don't exist at deferred call time.
    auto deferred2 = fnk::defer (&foo::bar, foo {"test"});

    std::cout << "calling deferred function with argument that no longer exists:" << std::endl;
    std::cout << deferred2() << std::endl;
    return 0;
}

