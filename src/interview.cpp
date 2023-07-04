#include <iostream>

struct Point final {
    constexpr Point(std::size_t row, std::size_t col) noexcept
        : row{row}, col{col} {}

    std::size_t row;
    std::size_t col;
};

std::ostream &operator<<(std::ostream &os, Point point) {
    return os << "Point{row: " << point.row << ", col: " << point.col << "}";
}

template<typename Consumer>
    requires std::invocable<Consumer, Point>
            && std::same_as<std::invoke_result_t<Consumer, Point>, void>
inline constexpr void
iter_diag_upper_halve_top_right_to_bottom_left_left_to_right(
        std::size_t width, std::size_t height, Consumer consumer
) noexcept(noexcept(std::declval<Consumer>()(std::declval<Point>()))) {
    for (std::size_t col = 0; col < width; ++col) {
        for (std::size_t row = 0; row < std::min(height, col + 1); ++row) {
            consumer(Point(row, col - row));
        }
    }
}

template<typename Consumer>
    requires std::invocable<Consumer, Point>
             && std::same_as<std::invoke_result_t<Consumer, Point>, void>
inline constexpr void
iter_diag_upper_halve_top_right_to_bottom_left_right_to_left(
        std::size_t width, std::size_t height, Consumer consumer
) noexcept(noexcept(std::declval<Consumer>()(std::declval<Point>()))) {
    const std::size_t max{std::numeric_limits<std::size_t>::max()};
    for (std::size_t col = width - 2; col != max; --col) {
        for (std::size_t row = 0; row < std::min(height, col + 1); ++row) {
            consumer(Point(row, col - row));
        }
    }
}
template<typename Consumer>
    requires std::invocable<Consumer, Point>
            && std::same_as<std::invoke_result_t<Consumer, Point>, void>
            && std::copy_constructible<Consumer>
constexpr void iterate_diagonal(
        std::size_t width, std::size_t height, Consumer consumer
) noexcept(noexcept(std::declval<Consumer>()(std::declval<Point>()))) {
    iter_diag_upper_halve_top_right_to_bottom_left_left_to_right(
            width, height, consumer);

    auto translate {
        [=](Point p) {
            consumer(Point{height - 1 - p.col, width - 1 - p.row});
        }
    };

    // Note: height and width are deliberately swapped.
    iter_diag_upper_halve_top_right_to_bottom_left_right_to_left(
            height, width, translate);
}


int main(const int argc, char const *const *const argv) {
    auto printPoint {[](Point p) {std::cout << p << '\n';}};

    std::size_t width{4};
    std::size_t height{3};

    iterate_diagonal(width, height, printPoint);
}
