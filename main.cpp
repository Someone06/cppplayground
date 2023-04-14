#include<ranges>
#include<string_view>

#include "src/ExitCode.h"

template<std::ranges::random_access_range R>
    requires std::same_as<std::ranges::range_value_t<R>, std::string_view>
[[nodiscard]] constexpr std::uint8_t newMain([[maybe_unused]] R args) noexcept;

int main(const int argc, char const * const * const argv) {
    auto toStringView = [](auto s) {return std::string_view{s};};
    auto args = std::views::counted(argv, argc)
                | std::views::transform(toStringView);
    return newMain(args);
}


enum class ExitCodes : ExitCode_t {Success = EXIT_SUCCESS, Failure = EXIT_FAILURE};

template<std::ranges::random_access_range R>
    requires std::same_as<std::ranges::range_value_t<R>, std::string_view>
[[nodiscard]] constexpr std::uint8_t newMain([[maybe_unused]] R args) noexcept {
    return *ExitCode(ExitCodes::Success);
}