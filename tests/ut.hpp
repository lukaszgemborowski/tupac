#ifndef UT_HPP
#define UT_HPP

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <sstream>

namespace ut
{
struct CondFailed : std::exception {
    CondFailed(std::string &&str)
        : reason {std::move(str)}
    {}

    const char* what() const noexcept {
        return reason.c_str();
    }

    std::string reason;
};

using TestCaseSignature = void (*)();

struct TestCase;
struct TestCaseInit {
    TestCaseInit(const char* name)
        : name {name} {
    }

    TestCase operator=(TestCaseSignature func);

    const char *name;
};

struct TestCase : TestCaseInit {
    TestCase(const char *name, TestCaseSignature func)
        : TestCaseInit{name}
    {
        try {
            func();
        } catch (CondFailed ex) {
            std::cout << name << " test failed at: " << ex.what() << std::endl;
        }
    }
};

TestCase TestCaseInit::operator=(TestCaseSignature func)
{
    return TestCase{name, func};
}

namespace literals
{
TestCaseInit operator"" _test(const char *name, std::size_t)
{
    return TestCaseInit{name};
}
}
}

#define check(EXPR) \
    do { \
        if (!(EXPR)) { \
            std::stringstream ss; \
            ss << "[" << __FILE__ << ":" << __LINE__ << "] failed" << std::endl; \
            throw ut::CondFailed{ss.str()}; \
        } \
    } while (0);

#endif // UT_HPP
