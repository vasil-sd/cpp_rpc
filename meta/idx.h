#pragma once

namespace meta {

namespace type_list {

template<typename T, int n, typename... Ts>
struct type_idx_;

template<typename T, int n, typename t, typename... Ts>
struct type_idx_<T, n, t, Ts...> : type_idx_<T, n+1, Ts...> {};

template<typename T, int n, typename... Ts>
struct type_idx_<T, n, T, Ts...> {
    constexpr static unsigned long long idx = n;
};

template<typename T, typename... Ts>
struct type_idx : type_idx_<T, 0, Ts...> {};

}
}
