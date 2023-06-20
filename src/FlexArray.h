#ifndef CPPPLAYGROUND_FLEXARRAY_H
#define CPPPLAYGROUND_FLEXARRAY_H

#include<cstddef>
#include<span>
#include<array>
#include<memory>
#include<stdexcept>

template<typename T, std::size_t N = std::dynamic_extent>
class FlexArray final {
public:
    [[nodiscard]] explicit constexpr FlexArray() = default;
    FlexArray(const FlexArray&) = delete;
    FlexArray& operator=(const FlexArray&) = delete;
    constexpr FlexArray(FlexArray&&) = default;
    constexpr FlexArray& operator=(FlexArray&&) = default;
    ~FlexArray() = default;


    [[nodiscard]] constexpr std::span<const T, N> get() const {
        return std::span(a);
    }

    [[nodiscard]] constexpr std::span<T, N> getMut() {
        return std::span(a);
    }

private:
    std::array<T, N> a {};
};

template<typename T>
class FlexArray<T, std::dynamic_extent> final {
public:
    [[nodiscard]] explicit constexpr FlexArray(std::size_t size) : ptr{new T[size]}, size{size} {}
    FlexArray(const FlexArray&) = delete;
    FlexArray& operator=(const FlexArray&) = delete;
    constexpr FlexArray(FlexArray&&) = default;
    constexpr FlexArray& operator=(FlexArray&&) = default;
    ~FlexArray() = default;

    [[nodiscard]] constexpr std::span<const T, std::dynamic_extent> get() const {
        return std::span(ptr.get(), size);
    }

    [[nodiscard]] constexpr std::span<T, std::dynamic_extent> getMut() {
        return std::span(ptr.get(), size);
    }

private:
    std::unique_ptr<T> ptr;
    std::size_t size;
};

template<typename T, std::size_t N>
requires (N != std::dynamic_extent)
[[nodiscard]] constexpr FlexArray<T, N> make_FlexArray(std::size_t size) {
    if(size != N)
        throw std::invalid_argument("Static and dynamic size do not match.");

    return FlexArray<T, N>{};
}

template<typename T, std::size_t N>
requires (N == std::dynamic_extent)
[[nodiscard]] constexpr FlexArray<T, std::dynamic_extent> make_FlexArray(std::size_t size) {
    return FlexArray<T, std::dynamic_extent>{size};
}

#endif
