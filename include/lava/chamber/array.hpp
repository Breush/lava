#pragma once

#include <array>
#include <cstdint>

namespace {
    template <typename T, std::size_t... Is>
    constexpr std::array<T, sizeof...(Is)> make_array_from_sequence(const T& value, std::index_sequence<Is...>)
    {
        return {{(static_cast<void>(Is), value)...}};
    }

    template <typename T, std::size_t... Is>
    constexpr std::array<T, sizeof...(Is)> make_array_from_sequence(T&& value, std::index_sequence<Is...>)
    {
        return {{(static_cast<void>(Is), std::forward<T>(value))...}};
    }
}

namespace lava::chamber {
    template <std::size_t N, typename T>
    constexpr std::array<T, N> make_array(const T& value)
    {
        return make_array_from_sequence(value, std::make_index_sequence<N>());
    }

    template <std::size_t N, typename T>
    constexpr std::array<T, N> make_array(T&& value)
    {
        return make_array_from_sequence(std::forward<T>(value), std::make_index_sequence<N>());
    }
}
