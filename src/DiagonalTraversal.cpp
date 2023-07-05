#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <limits>
#include <utility>

/*
 * Generates the coordinates needed to traverse an 2D array diagonally.
 * Width and height of the array do not need to be equal.
 * Assume the coordinate `(0, 0)` are in the top left and the coordinates
 * `(width - 1, height - 1)` in the bottom right.
 * Then the diagonals are traversed from to top left to bottom right, where each
 * diagonal is traversed from top right to bottom left.
 * A 3 by 4 array would be traversed in the following order:
 * ```
 * 0  1  3
 * 2  4  6
 * 5  7  9
 * 8 10 11
 * ```
 */

struct Point final {
    [[nodiscard]] constexpr Point(std::size_t row, std::size_t col) noexcept
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
        const std::size_t bound {std::min(height, col + 1)};
        for (std::size_t row = 0; row < bound; ++row) {
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
        const std::size_t bound {std::min(height, col + 1)};
        for (std::size_t row = 0; row < bound; ++row) {
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

void print_traversed_coordinates(std::size_t width, std::size_t height) {
    auto printPoint {[](Point p) {std::cout << p << '\n';}};
    iterate_diagonal(width, height, printPoint);
}

int main(const int argc, char const *const *const argv) {
    std::size_t width {4};
    std::size_t height {3};

    print_traversed_coordinates(width, height);
}
