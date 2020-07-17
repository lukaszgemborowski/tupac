#include "tupac.hpp"
#include <iostream>

struct aClass {};

int main()
{
    auto print = [](auto &e) { std::cout << e << ", "; };

    std::make_tuple(aClass{}, 1)
        | tupac::push_back(3.4)
        | tupac::remove_if(tupac::is_integral || tupac::is_class)
        | tupac::foreach(print);

    auto t3 = std::make_tuple(1, 2.3);
    auto t4 = t3 | tupac::mutate(tupac::make_reference);
    static_assert(std::is_same_v<decltype(t4), std::tuple<int&, double&>>);
}
