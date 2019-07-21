#pragma once

#include "meta/tuple.h"
#include "meta/idx.h"
#include <type_traits>
#include <tuple>

namespace hash {

using UI64 = unsigned long long;

// from murmur3
template<UI64 v>
class calc_hash{
    constexpr static UI64 v0 = v ^ (v >> 33);
    constexpr static UI64 v1 = v0 * 0xff51afd7ed558ccdLLU;
    constexpr static UI64 v2 = v1 ^ (v1 >> 33);
    constexpr static UI64 v3 = v2 * 0xc4ceb9fe1a85ec53LLU;
    constexpr static UI64 v4 = v3 ^ (v3 >> 33);
public:
    constexpr static UI64 value = v4;
};

template<typename...T>
struct type_hash;

//===========================================
// primitive types

template<>
struct type_hash<bool> : calc_hash<0x1> {};

template<>
struct type_hash<char> : calc_hash<0x2> {};

template<>
struct type_hash<int> : calc_hash<0x3> {};

template<>
struct type_hash<std::string> : calc_hash<0x10> {};

//===========================================
// Tuples

template <typename...>
struct type_hash_tup_;

template<typename T, typename...Ts, typename ...Ts1>
struct type_hash_tup_<std::tuple<Ts1...>, std::tuple<T, Ts...>> :
       type_hash_tup_<std::tuple<Ts1...>, std::tuple<Ts...>>
{
    using next = type_hash_tup_<std::tuple<Ts1...>, std::tuple<Ts...>>;
    constexpr static UI64 value = 
        (calc_hash<meta::type_list::type_idx<T, Ts1...>::idx + 1>::value * type_hash<T>::value) ^ next::value;
};

template<typename ...Ts1>
struct type_hash_tup_<std::tuple<Ts1...>, std::tuple<>> :
       calc_hash<0x55AA>
{ };

template<typename... Ts>
struct type_hash<std::tuple<Ts...>> :
       type_hash_tup_<std::tuple<Ts...>, std::tuple<Ts...>>
{ };


//===========================================
// structures

template <typename S>
struct struct_type_hash : type_hash<decltype(meta::tuple::to_tuple(S{}))> {};

#define STRUCT_HASH(S) \
namespace hash { \
template<> \
struct type_hash<S> : struct_type_hash<S> {}; \
}

#define SET_HASH(O, N) \
namespace hash { \
template<> \
struct type_hash<O> : calc_hash<N+0x100> {}; \
}

} // namespace hash
