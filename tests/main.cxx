#include <tupac.hpp>
#include "ut.hpp"

namespace {
struct aClass {
    int aValue;
};
}

using namespace ut::literals;

namespace {
template<class... Args, class B>
void is_same_tuple(const B&) {
    auto value = std::is_same_v<std::tuple<Args...>, B>;
    check(value);
}
}

auto algo_push_back = "algo::push_back"_test = []() {
    using namespace std::literals;

    auto t = std::make_tuple(1, 2.3);
    is_same_tuple<int, double>(t);

    auto t2 = tupac::algo::push_back<const char *>(t, "foo");
    is_same_tuple<int, double, const char*>(t2);

    auto t3 = tupac::algo::push_back(t2, aClass{42});
    is_same_tuple<int, double, const char*, aClass>(t3);

    check(std::get<0>(t3) == 1);
    check(std::get<1>(t3) == 2.3);
    check(std::get<2>(t3) == "foo"s);
    check(std::get<3>(t3).aValue == 42);
};

auto push_back = "push_back"_test = []() {
    auto t = std::make_tuple(42);
    auto t2 = t | tupac::push_back(3.14);

    is_same_tuple<int, double>(t2);
};

auto algo_for_each = "algo::for_each"_test = []() {
    auto t = std::make_tuple(1, 2.3, 4.5f);
    int result = 0;

    tupac::algo::for_each(
        t,
        [&](auto const& e) {
            result += static_cast<int>(e);
        }
    );

    check(result == 7);

    tupac::algo::for_each(
        t,
        [&](auto& e) {
            e += 1;
        }
    );

    check(std::get<0>(t) == 2);
    check(std::get<1>(t) == 3.3);
    check(std::get<2>(t) == 5.5f);
};

auto for_each = "for_each"_test = []() {
    auto t = std::make_tuple(1, 2, 3);
    int result = 0;

    t | tupac::for_each([&](auto const &e) { result += e; });

    check(result == 6);
};

int main() {}
