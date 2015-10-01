
#include <iostream>

#include "eval.hpp"

void function (int const& x)
{
    std::cout << "got a number: " << x << std::endl;
}

int main (void)
{
    int arg1 = 1;
    
    fnk::eval (function, arg1);
    std::cout << "just evaled 'function' with argument: " << arg1 << "." << std::endl;
   
    return 0;
}

