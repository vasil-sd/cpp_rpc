#pragma once

#include <type_traits>

namespace meta {

namespace tuple {

namespace {

template <typename T, typename... TArgs>
decltype(void(T{std::declval<TArgs>()...}), std::true_type{}) test_is_braces_constructible(int);

template <typename, typename...> std::false_type test_is_braces_constructible(...);

template <typename T, typename... TArgs>
using is_braces_constructible = decltype(test_is_braces_constructible<T, TArgs...>(0));

struct any {
  template<class T>
  constexpr operator T(); // non explicit
};

} // namespace anonymous

template<class T>
auto to_tuple(T&& object) noexcept {
    using type = std::decay_t<T>;
    if constexpr(is_braces_constructible<type, any, any, any, any, any, any>{}) {
      auto&& [p1, p2, p3, p4, p5, p6] = object;
      return std::make_tuple(p1, p2, p3, p4, p5, p6);
    } else if constexpr(is_braces_constructible<type, any, any, any, any, any>{}) {
      auto&& [p1, p2, p3, p4, p5] = object;
      return std::make_tuple(p1, p2, p3, p4, p5);
    } else if constexpr(is_braces_constructible<type, any, any, any, any>{}) {
      auto&& [p1, p2, p3, p4] = object;
      return std::make_tuple(p1, p2, p3, p4);
    } else if constexpr(is_braces_constructible<type, any, any, any>{}) {
      auto&& [p1, p2, p3] = object;
      return std::make_tuple(p1, p2, p3);
    } else if constexpr(is_braces_constructible<type, any, any>{}) {
      auto&& [p1, p2] = object;
      return std::make_tuple(p1, p2);
    } else if constexpr(is_braces_constructible<type, any>{}) {
      auto&& [p1] = object;
      return std::make_tuple(p1);
    } else {
        return std::make_tuple();
    }
}

template<class T, typename...Args>
T from_tuple(std::tuple<Args...>&& tuple) noexcept {
    if constexpr(sizeof...(Args) == 6) {
        auto&& [p1, p2, p3, p4, p5, p6] = tuple;
        return T{p1, p2, p3, p4, p5, p6};
    } else if constexpr(sizeof...(Args) == 5) {
        auto&& [p1, p2, p3, p4, p5] = tuple;
        return T{p1, p2, p3, p4, p5};
    } else if constexpr(sizeof...(Args) == 4) {
        auto&& [p1, p2, p3, p4] = tuple;
        return T{p1, p2, p3, p4};
    } else if constexpr(sizeof...(Args) == 3) {
        auto&& [p1, p2, p3] = tuple;
        return T{p1, p2, p3};
    } else if constexpr(sizeof...(Args) == 2) {
        auto&& [p1, p2] = tuple;
        return T{p1, p2};
    } else if constexpr(sizeof...(Args) == 1) {
        auto&& [p1] = tuple;
        return T{p1};
    } else {
        return T{};
    }

}

} // namespace tuple
} // namespace meta
