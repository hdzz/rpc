
#include <iostream>
#include <list>
#include <string>

#include "mappable.hpp"
#include "monoid.hpp"
#include "zip.hpp"

decltype(auto) repeat_string (std::string const& s, std::size_t const n)
{
    return fnk::repeat (s, n);
}

decltype(auto) repeat_string_tup (std::tuple<std::string const, std::size_t const> & t)
{
    return repeat_string (std::get<0>(t), std::get<1>(t));
}

int main (void)
{
    std::list<std::size_t const> li { 0, 1, 2, 3 };
    std::list<std::string const> ls { "nothing", "test", "string", "repeat" };

    //
    // Two ways to perform the same operation
    //
    for (auto const& e : fnk::map (&repeat_string_tup, fnk::zip (ls, li)))
        std::cout << e << std::endl;

    for (auto const& e : fnk::zip_with (&repeat_string, ls, li))
        std::cout << e << std::endl; 
    
    return 0;
}

