#pragma once

namespace meta {

template<template<typename> typename F, typename...Ts>
struct for_each;

template<template<typename> typename F, typename T, typename...Ts>
struct for_each<F, T, Ts...> {
    using next = for_each<F, Ts...>;
    template<typename... Args>
    static void call(Args&&...args) {
        F<T>{}(std::forward<Args>(args)...);
        next::call(std::forward<Args>(args)...);
    }
};

template<template<typename> typename F>
struct for_each<F> {
    template<typename...Args>
    static void call(Args&&...) { };
};

}
