#ifndef CPPPLAYGROUND_EXITCODE_H
#define CPPPLAYGROUND_EXITCODE_H

#include <concepts>
#include <cstdint>
#include <type_traits>

#include "enums.h"

using ExitCode_t = std::uint8_t;

template<class Enum>
    requires std::is_enum_v<Enum>
            && std::same_as<
                    std::invoke_result_t<
                            decltype([](Enum e){return to_underlying(e);}),
                            Enum>,
                    ExitCode_t>
class [[nodiscard]] ExitCode final {
public:
    explicit constexpr ExitCode(Enum exitCode) noexcept : c{exitCode} {}

    [[nodiscard]] inline constexpr ExitCode_t getValue() const noexcept {
        return to_underlying(c);
    }

    [[nodiscard]] inline constexpr ExitCode_t operator*() const noexcept {
        return getValue();
    }

    [[nodiscard]] inline constexpr Enum getExitCode() const noexcept {
        return c;
    }

    [[nodiscard]] inline constexpr bool operator==(ExitCode rhs) const noexcept {
        return c == rhs.c;
    }

    [[nodiscard]] inline constexpr bool operator!=(ExitCode rhs) const noexcept {
        return rhs != *this;
    }

private:
    Enum c;
};
#endif
