#ifndef MATH_HPP
#define MATH_HPP

#include <concepts>
#include <stdexcept>
#include <type_traits>

template <typename T>
concept number = std::integral<T> || std::floating_point<T>;

class CalculatorMath {
public:
    template <number T, number... Rest>
    constexpr static auto Add(T first, Rest... rest) noexcept {
        return (first + ... + rest);
    }

    template <number T, number... Rest>
    constexpr static auto Subtract(T first, Rest... rest) noexcept {
        if constexpr (sizeof...(rest) == 0) {
            return -first;
        }
        else {
            return (first - ... - rest);
        }
    }

    template <number T, number... Rest>
    constexpr static auto Multiply(T first, Rest... rest) noexcept {
        return (first * ... * rest);
    }

    template <number T, number... Rest>
    constexpr static auto Divide(T first, Rest... rest) {
        if constexpr (sizeof...(rest) == 0) {
            return static_cast<double>(first);
        }
        else {
            if (((rest == 0) || ...)) {
                throw std::invalid_argument("Division by zero");
            }

            using CommonType = std::common_type_t<T, Rest...>;
            if constexpr (std::integral<CommonType>) {
                return (static_cast<double>(first) / ... / static_cast<double>(rest));
            }
            else {
                return (first / ... / rest);
            }
        }
    }
};

#endif
