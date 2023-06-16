#ifndef CPPPLAYGROUND_RANGEHELPER_H
#define CPPPLAYGROUND_RANGEHELPER_H

#include <concepts>
#include <ranges>

template<std::ranges::random_access_range R>
    requires std::ranges::sized_range<R>
[[nodiscard]] constexpr auto left_half(R &r) {
    return std::ranges::subrange(std::begin(r), std::begin(r) + std::ranges::size(r) / 2);
}

template<std::ranges::random_access_range R>
    requires std::ranges::sized_range<R>
[[nodiscard]] constexpr auto right_half(R &r) {
    return std::ranges::subrange(std::begin(r) + std::ranges::size(r) / 2, std::end(r));
}

#endif
