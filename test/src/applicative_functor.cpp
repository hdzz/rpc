
#include <iostream>

#include "applicative_functor.hpp"
#include "type_support/maybe.hpp"

using namespace fnk;

int main (void)
{
    auto plus_3_m = type_support::make_maybe ([](int x) { return x + 3; });
    auto times_3_m = type_support::make_maybe ([](int x) { return x * 3; });

    std::cout << "Just (+3) `apply` Just (9): "
        << applicative_functor<type_support::maybe<int>>::apply (plus_3_m, type_support::make_maybe (9)).just_or (0)
        << std::endl;

    std::cout << "Just (*3) `apply` Just (10): "
        << applicative_functor<type_support::maybe<int>>::apply (times_3_m, type_support::make_maybe (10)).just_or (0)
        << std::endl;

    return 0;
}

