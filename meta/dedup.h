#pragma once

#include <type_traits>

namespace meta {

namespace dedup {

template<typename needle, typename...haystack>
constexpr bool is_type_present()
{
  return (std::is_same<needle, haystack>::value || ...);
}

template <template<typename...> typename, typename...> struct dedup_template_args;

template<template<typename...> typename T, typename t, typename ... list, typename ... result>
struct dedup_template_args<T, T<t, list...>, T<result...>>
{
  using type= typename
    std::conditional<
      is_type_present<t, list...>(),
      typename dedup_template_args<
        T,
        T<list...>,
        T<result...>
      >::type,
      typename dedup_template_args<
        T,
        T<list...>,
        T<result..., t>
      >::type
    >::type;
};
 
template<template<typename...> typename T, typename ... list>
struct dedup_template_args<T, T<>, T<list...>>
{
  using type = T<list...>;
};

template<template<typename...> typename T, typename ... list>
struct dedup_template_args<T, T<list...>>
{
  using type = typename dedup_template_args<T, T<list...>, T<>>::type;
};

}
}