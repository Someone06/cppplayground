#include<type_traits>
#include<bit>
#include <iostream>

/*
 * Test: Can Relu be implemented more efficiently than just using std::max by
 * making use of  bit twiddling?
 *
 * Result: No, both implementations get compiled the same assembly, which in
 * case of int on clang 16.0.0 using -O3 and compiling to x86_64 looks like this:
 *
 * xor     eax, eax
 * test    edi, edi
 * cmovg   eax, edi
 * ret
 *
 * So the compiler simply uses a test which sets a status register if the input
 * is negative and uses a conditional move to compute the result. Compilers
 * really do some magical stuff.
 */

template<typename T>
concept plain_signed = std::is_signed_v<T> && (!std::is_const_v<T>) && (!std::is_volatile_v<T>);

template<plain_signed T>
[[nodiscard]]constexpr T relu_using_max(T n) noexcept {
    return std::max<T>(n, T(0));
}

template<plain_signed T>
[[nodiscard]]constexpr T relu_bit_twiddle(T n) noexcept {
    using U = std::make_unsigned_t<T>;
    U unsigend {std::bit_cast<U, T>(n)};
    bool is_negative {std::signbit(n)};
    U zero {0};
    U mask {zero - U(!is_negative)};
    return unsigend & mask;
}

int main() {
    auto is_correct {
        [](plain_signed auto x) {
            return relu_bit_twiddle(x) == relu_using_max(x);
        }
    };

    std::cout << std::boolalpha << is_correct(1) << std::endl;
}
