//
// A small example of algebraic data type usage
//

#include <iostream>
#include <list>

#include "filterable.hpp"
#include "mappable.hpp"
#include "algebraic.hpp"

int main (void)
{
    using data_t = fnk::adt<int, std::string>; 
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
 
    auto int_entry = [] (data_t const& e) { return data_t::index<int>::value         == e.type_index(); };   
    auto str_entry = [] (data_t const& e) { return data_t::index<std::string>::value == e.type_index(); }; 

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

