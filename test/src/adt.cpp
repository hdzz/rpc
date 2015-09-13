//
// A small expression evalutaion interpreter
//

#include <iostream>
#include <list>

#include "filterable.hpp"
#include "mappable.hpp"
#include "type_support/algebraic.hpp"
#include "type_support/fix.hpp"

template <typename T>
decltype(auto) foo (T t)
{
    return t; 
}

int main (void)
{
    using data_t = fnk::type_support::adt<int, std::string>; 
    std::list<data_t> inputs;

    std::string s;
    while (std::cin >> s)
    {
        bool is_int = true;
        for (auto const& c : s)
           is_int = is_int && std::isdigit(c);
        if (is_int)
            inputs.push_back (data_t::emplace<int>(std::stoi(s)));
        else
            inputs.push_back (data_t::emplace<std::string>(s));
    }

    auto int_entry = [] (data_t const& e) { return 0 == e.tindex(); };   
    auto str_entry = [] (data_t const& e) { return 1 == e.tindex(); }; 

    auto square = [] (data_t const& e) { return e.template value<int>() * e.template value<int>(); };

    std::cout << "the numbers entered were:" << std::endl;
    for (auto const& e : fnk::filter (int_entry, inputs))
        std::cout << "\t" << e.template value<int>() << std::endl;

    std::cout << "the numbers entered squared:" << std::endl;
    for (auto const& e : fnk::map (square, fnk::filter (int_entry, inputs)))
        std::cout << "\t" << e << std::endl;

    std::cout << "the strings entered were:" << std::endl; 
    for (auto const& e : fnk::filter (str_entry, inputs))
        std::cout << "\t" << e.template value<std::string>() << std::endl;
    
    return 0;
}

