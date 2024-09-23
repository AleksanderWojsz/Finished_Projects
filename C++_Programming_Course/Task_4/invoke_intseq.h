#ifndef INVOKE_INTSEQ_H
#define INVOKE_INTSEQ_H

#include <iostream>
#include <functional>
#include <tuple>
#include <utility>
#include <type_traits>

namespace detail {

template <class T> // Default template => everything "fits"
struct is_this_int_seq : std::false_type {};

template <class T, T... args> // Template, in which only integer_sequence "fits"
struct is_this_int_seq<std::integer_sequence<T, args...>> : std::true_type {};

template <class... args>
constexpr bool is_any_int_seq() {
    return (is_this_int_seq<std::decay_t<args>>::value || ...);
}

// Gives the return type of the given function 'f'.
template <class Tuple, class F, class Arg, class... Args>
constexpr decltype(auto) check_function_type(F&& f, Tuple&& tuple, Arg&& arg, Args&&... args) {
    if constexpr (!is_this_int_seq<std::decay_t<Arg>>::value) {
        if constexpr (sizeof...(Args) > 0) { // If this is not the last argument, we do the same thing for the next one.
            return check_function_type<>(std::forward<F>(f), std::tuple_cat(std::forward<Tuple>(tuple),
                                                                            std::forward_as_tuple(std::forward<Arg>(arg))),std::forward<Args>(args)...);
        } else { // If this is the last argument, we invoke the function.
            return std::apply(std::forward<F>(f), std::tuple_cat(std::forward<Tuple>(tuple),
                                                                 std::forward_as_tuple(std::forward<Arg>(arg))));
        }
    } else {
        // If the argument is an integer_sequence, we invoke substitute it with any int.
        return check_function_type<>(std::forward<F>(f), std::forward<Tuple>(tuple),
                                     std::integral_constant<size_t, 1>{}, std::forward<Args>(args)...);
    }
}

// Gives the return type of the given function 'f' (if 'f' takes no arguments).
template <class Tuple, class F>
constexpr decltype(auto) check_function_type(F&& f, Tuple&& tuple) {
    return std::apply(std::forward<F>(f), tuple);
}

// Function declaration (used in iterate_over_int_seq):
template <bool is_void, class vector_t, class Tuple, class F, class Arg, class... Args>
constexpr void create_arguments_list(std::vector<vector_t> &results, F&& f, Tuple&& tuple, Arg&& arg, Args&&... args);

// Function for iterating over an integer_sequence
template <bool is_void, class vector_t, class F, class Tuple, class T, T... ints, class... Args>
constexpr void iterate_over_int_seq(std::vector<vector_t> &results, F&& f, Tuple&& tuple, std::integer_sequence<T, ints...>, Args&&... args) {
    (..., (create_arguments_list<is_void, vector_t>(results, std::forward<F>(f),
                                                         std::forward<Tuple>(tuple), std::integral_constant<T, ints>{}, std::forward<Args>(args)...)));
}

// Function for applying given function f to all elements in nearest integer_sequence:
// (adds arguments to a tuple until an integer_sequence is encountered => then it iterates over it)
template <bool is_void, class vector_t, class Tuple, class F, class Arg, class... Args>
constexpr void create_arguments_list(std::vector<vector_t> &results, F&& f, Tuple&& tuple, Arg&& arg, Args&&... args) {
    if constexpr (!is_this_int_seq<std::decay_t<Arg>>::value) {
        if constexpr (sizeof...(Args) > 0) { // If this is not the last argument, we do the same thing for the next one.
            create_arguments_list<is_void, vector_t>(results, std::forward<F>(f),
                                                          std::tuple_cat(std::forward<Tuple>(tuple), std::forward_as_tuple(std::forward<Arg>(arg))), std::forward<Args>(args)...);
        } else {
            if constexpr (!is_void) { // If function type is not void, we are saving the result.
                results.push_back(std::apply(std::forward<F>(f), std::tuple_cat(std::forward<Tuple>(tuple),
                                                                                std::forward_as_tuple(std::forward<Arg>(arg)))));
            } else { // Just calling the function.
                std::apply(std::forward<F>(f), std::tuple_cat(std::forward<Tuple>(tuple),
                                                              std::forward_as_tuple(std::forward<Arg>(arg))));
            }
        }
    } else { // if next argument is an integer_sequence we are iterating over it.
        iterate_over_int_seq<is_void, vector_t>(results, std::forward<F>(f),
                                                std::forward<Tuple>(tuple), arg, std::forward<Args>(args)...);
    }
}

// For invoking 'invoke_intseq' with at least one argument:
template <bool is_void, class vector_t, class F, class Arg, class... Args>
constexpr void run_function(std::vector<vector_t> &results, F&& f, Arg&& arg, Args&&... args) {
    return create_arguments_list<is_void, vector_t>(results, std::forward<F>(f),
                                                         std::make_tuple(), std::forward<Arg>(arg), std::forward<Args>(args)...);
}

// For invoking 'invoke_intseq' without arguments:
template <bool is_void, class vector_t, class F>
constexpr void run_function(std::vector<vector_t> &results, F&& f) {
    if constexpr (!is_void) { // If function type is not void, we are saving the result.
        results.push_back(std::apply(std::forward<F>(f), std::make_tuple()));
    }
    std::apply(std::forward<F>(f), std::make_tuple());
}

} // namespace detail

template <class F, class... Args>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {

    using function_t = decltype(detail::check_function_type(std::forward<F>(f), std::make_tuple(), std::forward<Args>(args)...));

    constexpr bool is_void = std::is_void_v<function_t>;
    constexpr bool contains_any_int_seq = detail::is_any_int_seq<Args...>();
    constexpr bool is_ref = std::is_reference_v<function_t>;

    if constexpr (!contains_any_int_seq) { // Standard function call.
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }
    else if constexpr (!is_void) { // We need to save all results.
        using ref_wrapper_t  = typename
        std::conditional<is_ref, std::reference_wrapper<std::remove_reference_t<function_t>>, function_t>::type;
        std::vector<ref_wrapper_t > result;
        detail::run_function<is_void, ref_wrapper_t >(result, std::forward<F>(f), std::forward<Args>(args)...);
        return result;
    }
    else { // Standard function call, but with integer_sequence.
        using foo_type = size_t;
        std::vector<foo_type> foo_vector;
        detail::run_function<is_void, foo_type>(foo_vector, std::forward<F>(f), std::forward<Args>(args)...);
        return;
    }
}

#endif //INVOKE_INTSEQ_H
