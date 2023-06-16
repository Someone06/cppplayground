#ifndef CPPPLAYGROUND_FLOATINGPOINTHELPER_H
#define CPPPLAYGROUND_FLOATINGPOINTHELPER_H

#include<concepts>
#include<numeric>

template<typename F>
concept plain_floating_point = std::floating_point<F> && !std::is_const_v<F> && !std::is_volatile_v<F>;

template<plain_floating_point F>
[[nodiscard]] constexpr bool is_approx_one(F x) {
    auto epsilon{std::numeric_limits<F>::epsilon()};
    return x >= 1.0 && (x - 1.0) <= epsilon || x < 1.0 && x + epsilon >= 1.0;
}

template<plain_floating_point F>
[[nodiscard]] inline constexpr bool is_between_zero_and_one_inclusive(F x) {
    return x >= 0.0 && x <= 1.0;
}
#endif
