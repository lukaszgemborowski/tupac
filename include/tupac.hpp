#ifndef TUPAC_TUPAC_HPP
#define TUPAC_TUPAC_HPP

#include <tuple>
#include <type_traits>

#if __cplusplus < 201703L
# error "tupac requires at least C++17 support"
#else

/// \brief Main tupac namespace
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

/// \brief Tuple algorithms.
///
/// This namespace contains all basic building blocks for a higher
/// level "syntactic suger" features of this library. You may want
/// to use functions from this namespace if you prefer.
namespace algo
{
/// \defgroup algo Algorithms
/// Basic tuple algorithms.
/// \{

/// \brief Execute a functor over each tuple element.
///
/// \param[in] tup input tuple
/// \param[in] fun user provided functor
///
/// Invoke a functor passing a reference to each tuple element.
/// Functor should accept a generic parameter type and/or
/// handle each type contained in the tuple.
/// \code{.cpp}
/// auto tup = make_tuple(1, "foo");
/// algo::for_each(
///     tup,
///     [](auto const& e) {
///         cout << e << ", ";
///     }
/// );
/// \endcode
template<class Tuple, class Fun>
void for_each(Tuple &tup, Fun fun)
{
    std::apply([&](auto&... args) { (fun(args), ...); }, tup);
}

/// \brief Remove elements that satisfy predicate
///
/// \param[in] tup input tuple
/// \param[in] fun user provided predicate
///
/// Creates a new tuple that does not contain elements
/// matching provided predicate.
/// \code{.cpp}
/// auto t = std::make_tuple(1, 2.2, 3, "");
/// auto t2= algo::remove_if(is_integral)
/// static_assert(is_same_v<decltype(t2), std::tuple<double, const char*>>)
/// \endcode
/// \see pred
template<class Tuple, class Fun>
auto remove_if(Tuple const& tup, Fun fun)
{
    auto negate = [fun](auto arg) { return !fun(arg); };
    return detail::tuple_from_indices(tup, detail::make_index_sequence_of(tup, negate));
}

/// \brief Change type and/or value of each tuple element
///
/// \param[in] tup input tuple
/// \param[in] fun mutating function
///
/// Translates every tuple element according to provided functor in
/// such a way that new tuple element have type of decltype(fun(tuple_element))
/// and a value of fun(tuple_element). A good example is to transform a tuple
/// of integers into tuple of references to the original tuple elements.
///
/// \code{.cpp}
/// auto t = std::make_tuple(1, 2.3);
/// auto t2 = algo::mutate(t, make_reference);
/// static_assert(is_same_v<decltype(t2), tuple<int &, double &>>);
/// \endcode
///
/// \see make_reference
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

/// \brief Add an element at the end of the tuple
///
/// \param[in] tup input tuple
/// \param[in] value value to append
///
/// Creates new tuple and append T at the end.
template<class T, class... Args>
auto push_back(std::tuple<Args...> const& tup, T&& value)
{
    return std::tuple_cat(tup, std::make_tuple(value));
}

/// \}

} // algo

namespace detail
{
template<class Fun>
struct foreach_t {
    template<class Tup>
    auto operator()(Tup&& tup) {
        algo::for_each(tup, fun_);
        return tup;
    }

    Fun fun_;
};

template<class T>
struct push_back_t {
    template<class... Args>
    auto operator()(std::tuple<Args...> const& tup) {
        return algo::push_back(tup, arg_);
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

template<class Rel, class A, class B>
struct logical_relation {
    template<class T>
    constexpr auto operator()(T t) const {
        return Rel()(A()(t), B()(t));
    }
};

template<template<typename> typename A, template<typename> typename B>
auto operator && (std_trait_adapter<A>, std_trait_adapter<B>) {
    return logical_relation<std::logical_and<>, std_trait_adapter<A>, std_trait_adapter<B>>{};
}

template<template<typename> typename A, template<typename> typename B>
auto operator || (std_trait_adapter<A>, std_trait_adapter<B>) {
    return logical_relation<std::logical_or<>, std_trait_adapter<A>, std_trait_adapter<B>>{};
}

} // detail

// operations

/// \defgroup func Modifiers
/// \{

/// \brief Execute a functor over each tuple element.
///
/// Executes algo::for_each over a tuple.
///
/// \code{.cpp}
/// a_tuple | foreach([](auto &e) { std::cout << e << ", "; });
/// \endcode
inline detail::binder_t<detail::foreach_t> foreach;

/// \brief Add an element at the end of the tuple
///
/// Executes algo::push_back over a tuple.
///
/// \code{.cpp}
/// auto t = std::make_tuple("foo");
/// auto u = t | push_back(42);
/// \endcode
inline detail::binder_t<detail::push_back_t> push_back;

/// \brief Remove elements that satisfy predicate
///
/// Executes algo::remove_if over a tuple.
///
/// \code{.cpp}
/// auto t = std::make_tuple(1, 2.2, 3, "");
/// auto t2 = t | remove_if(is_integral)
/// static_assert(is_same_v<decltype(t2), std::tuple<double, const char*>>)
/// \endcode
/// \see pred
inline detail::binder_t<detail::remove_if> remove_if;

/// \brief Change type and/or value of each tuple element
///
/// Executes algo::mutate over a tuple.
///
/// \code{.cpp}
/// auto t = std::make_tuple(1, 2.3);
/// auto t2 = t | mutate(make_reference);
/// static_assert(is_same_v<decltype(t2), tuple<int &, double &>>);
/// \endcode
///
/// \see make_reference
inline detail::binder_t<detail::mutate_t> mutate;
/// \}

/// \defgroup pred Predicates
/// \{

/// \brief Wraps std::is_integral trait
inline detail::std_trait_adapter<std::is_integral> is_integral;

/// \brief Wraps std::is_floating_point trait
inline detail::std_trait_adapter<std::is_floating_point> is_floating_point;

/// \brief Wraps std::is_array trait
inline detail::std_trait_adapter<std::is_array> is_array;

/// \brief Wraps std::is_enum trait
inline detail::std_trait_adapter<std::is_enum> is_enum;

/// \brief Wraps std::is_union trait
inline detail::std_trait_adapter<std::is_union> is_union;

/// \brief Wraps std::is_class trait
inline detail::std_trait_adapter<std::is_class> is_class;

/// \brief Wraps std::is_function trait
inline detail::std_trait_adapter<std::is_function> is_function;

/// \brief Wraps std::is_pointer trait
inline detail::std_trait_adapter<std::is_pointer> is_pointer;

/// \brief Wraps std::is_lvalue_reference trait
inline detail::std_trait_adapter<std::is_lvalue_reference> is_lvalue_reference;

/// \brief Wraps std::is_rvalue_reference trait
inline detail::std_trait_adapter<std::is_rvalue_reference> is_rvalue_reference;

/// \brief Wraps std::is_member_object_pointer trait
inline detail::std_trait_adapter<std::is_member_object_pointer> is_member_object_pointer;

/// \brief Wraps std::is_member_function_pointer trait
inline detail::std_trait_adapter<std::is_member_function_pointer> is_member_function_pointer;

/// \brief Wraps std::is_fundamental trait
inline detail::std_trait_adapter<std::is_fundamental> is_fundamental;

/// \brief Wraps std::is_arithmetic rait
inline detail::std_trait_adapter<std::is_arithmetic> is_arithmetic;

/// \brief Wraps std::is_scalar trait
inline detail::std_trait_adapter<std::is_scalar> is_scalar;

/// \brief Wraps std::is_object trait
inline detail::std_trait_adapter<std::is_object> is_object;

/// \brief Wraps std::is_compound trait
inline detail::std_trait_adapter<std::is_compound> is_compound;

/// \brief Wraps std::is_reference trait
inline detail::std_trait_adapter<std::is_reference> is_reference;

/// \brief Wraps std::is_member_pointer trait
inline detail::std_trait_adapter<std::is_member_pointer> is_member_pointer;

/// \}

// TODO: type properties, supported operations
// TODO: new adapter required: type relationships

// mutators
struct make_reference_t {
    template<class T>
    T& operator()(T&& element) const {
        return element;
    }
};

inline make_reference_t make_reference;

}

#endif // __cplusplus
#endif // TUPAC_TUPAC_HPP
