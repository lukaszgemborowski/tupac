#include "tupac.hpp"

struct aClass {};

int main()
{
    auto t = std::make_tuple(1, 2, "foo", 3, 3.14, aClass{});
    auto t2 = t | tupac::remove_if(tupac::is_integral || tupac::is_class);
    static_assert(std::is_same_v<decltype(t2), std::tuple<const char *, double>>);
}
