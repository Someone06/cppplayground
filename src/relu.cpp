#include<type_traits>
#include<bit>
#include<cmath>
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
 *
 * So the compiler simply uses a test which sets a status register if the input
 * is negative and uses a conditional move to compute the result. GCC produces
 * the same code in both cases as well, but makes uses of the wrapping behaviour
 * of unsigned integers instead and is essentially the same as the manually
 * written bit twiddling solution:
 *
 * mov     eax, edi
 * shr     eax, 31
 * sub     eax, 1
 * and     eax, edi
 *
 * Compilers really do some magical stuff.
 */

template<typename T>
concept plain_signed = std::is_signed_v<T> && (!std::is_const_v<T>)
                                           && (!std::is_volatile_v<T>);

template<plain_signed T>
[[nodiscard]]constexpr T relu_using_max(T n) noexcept {
    return std::max<T>(n, T(0));
}

template<plain_signed T>
[[nodiscard]]constexpr T relu_bit_twiddle(T n) noexcept {
    using U = std::make_unsigned_t<T>;
    U n_unsigned {std::bit_cast<U, T>(n)};
    bool is_negative {std::signbit(n)};
    U mask {U(0) - U(!is_negative)};
    return n_unsigned & mask;
}

int main() {
    auto is_correct {
        [](plain_signed auto x) -> bool {
            return relu_bit_twiddle(x) == relu_using_max(x);
        }
    };

    std::cout << std::boolalpha << is_correct(-1) << std::endl;
}
