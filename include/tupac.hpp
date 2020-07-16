#ifndef TUPAC_TUPAC_HPP
#define TUPAC_TUPAC_HPP

#include <tuple>
#include <type_traits>

#if __cplusplus < 201703L
# error "tupac requires at least C++17 support"
#else

namespace tupac
{
namespace detail
{

template<class T>
struct type_ {
    using type = T;
};

template<std::size_t I, class Fun, class Tuple, std::size_t... Indices>
constexpr auto detail_make_index_sequence_of(Tuple tuple, Fun fun, std::integer_sequence<std::size_t, Indices...> seq)
{
    if constexpr(std::tuple_size_v<Tuple> == I) {
        return seq;
    } else {
        if constexpr (fun(type_<std::tuple_element_t<I, Tuple>>{})) {
            return detail_make_index_sequence_of<I + 1>(tuple, fun, std::integer_sequence<std::size_t, Indices..., I>{});
        } else {
            return detail_make_index_sequence_of<I + 1>(tuple, fun, seq);
        }
    }
}

template<class Tuple, class Fun>
constexpr auto make_index_sequence_of(Tuple tuple, Fun fun)
{
    return detail_make_index_sequence_of<0>(tuple, fun, std::integer_sequence<std::size_t>{});
}

template<class Tuple, std::size_t... Indices>
constexpr auto tuple_from_indices(Tuple tuple, std::integer_sequence<std::size_t, Indices...>)
{
    return std::tuple<std::tuple_element_t<Indices, Tuple>...> {
        std::get<Indices>(tuple)...
    };
}

} // detail


namespace algo
{
template<class Tuple, class Fun>
void for_each(Tuple &tup, Fun fun)
{
    std::apply([&](auto&... args) { (fun(args), ...); }, tup);
}

template<class Tuple, class Fun>
auto remove_if(Tuple const& tup, Fun fun)
{
    auto negate = [fun](auto arg) { return !fun(arg); };
    return detail::tuple_from_indices(tup, detail::make_index_sequence_of(tup, negate));
}

template<class Tuple, class Fun>
auto mutate(Tuple&& tup, Fun fun)
{
    // Tuple is either "const Tuple&" or "Tuple&"
    // and we can only accept non-const lvalue references
    static_assert(std::is_lvalue_reference<Tuple>::value, "Tuple is not lvalue reference. Typically you want to mutate before applying any other intrusive operators");
    return std::apply(
        [&](auto&&... args) {
            return std::tuple<decltype(fun(args))...>{fun(args)...};
        },
        tup
    );
}

}

namespace detail
{
template<class Fun>
struct foreach_t {
    template<class Tup>
    auto operator()(Tup &tup) {
        algo::for_each(tup, fun_);
        return tup;
    }

    Fun fun_;
};

template<class T>
struct push_back_t {
    template<class... Args>
    auto operator()(std::tuple<Args...> &tup) {
        return std::apply(
            [&, this](auto&... args) { return std::tuple<Args..., T>{args..., arg_}; },
            tup
        );
    }

    T arg_;
};

template<class Fun>
struct remove_if {
    template<class Tup>
    auto operator()(Tup const& tup) {
        return algo::remove_if(tup, fun_);
    }

    Fun fun_;
};

template<class Fun>
struct mutate_t {
    template<class Tup>
    auto operator()(Tup&& tup) {
        return algo::mutate(std::forward<Tup>(tup), fun_);
    }

    Fun fun_;
};

template<template<typename...> typename T>
struct binder_t {
    template<class Fun>
    auto operator()(Fun fun) {
        return T<Fun>{fun};
    }
};

template<class Tuple, class Functor>
auto operator|(Tuple&& tuple, Functor functor)
{
    return functor(std::forward<Tuple>(tuple));
}

template<template<typename> typename Trait>
struct std_trait_adapter {
    template<class T>
    constexpr auto operator()(T) const {
        return Trait<typename T::type>::value;
    }
};

} // detail

inline detail::binder_t<detail::foreach_t> foreach;
inline detail::binder_t<detail::push_back_t> push_back;
inline detail::binder_t<detail::remove_if> remove_if;
inline detail::binder_t<detail::mutate_t> mutate;

inline detail::std_trait_adapter<std::is_integral> is_integral;

}

#endif // __cplusplus
#endif // TUPAC_TUPAC_HPP
