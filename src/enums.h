#ifndef CPPPLAYGROUND_ENUMS_H
#define CPPPLAYGROUND_ENUMS_H

#include <type_traits>
#include <utility>

template<class Enum>
    requires std::is_enum_v<Enum>
[[nodiscard]] inline constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
    return static_cast<decltype(to_underlying(e))>(e);
}

#endif
