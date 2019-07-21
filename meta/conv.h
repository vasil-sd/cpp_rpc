#pragma once

#include "meta/idx.h"

namespace meta {

namespace tuple {

template<int... inds>
struct indices {};

template<int i, int... inds>
indices<inds..., i> append_idx(indices<inds...>);

template<typename...>
struct converter;

template<typename... Tl1, typename... Tl3, typename T, typename... Tl2, typename inds>
struct converter<std::tuple<Tl1...>,std::tuple<Tl3...>, std::tuple<T, Tl2...>, inds> : 
    converter<std::tuple<Tl1...>,std::tuple<Tl3...>, std::tuple<Tl2...>, decltype(append_idx<meta::type_list::type_idx<T, Tl1...>::idx>(inds{})) >{};


template<typename... Tl1, typename... Tl3, typename inds>
struct converter<std::tuple<Tl1...>, std::tuple<Tl3...>, std::tuple<>, inds> {
    template<int... indxs>
    static std::tuple<Tl3...> convert_(std::tuple<Tl1...>&& arg, indices<indxs...>){
        return {std::move(std::get<indxs>(arg))...};
    }
    static std::tuple<Tl3...> convert(std::tuple<Tl1...>&& arg){
        return convert_(std::move(arg), inds{});
    }
};

template<typename... Tl1, typename... Tl2>
struct converter<std::tuple<Tl1...>, std::tuple<Tl2...>> :
  converter<std::tuple<Tl1...>, std::tuple<Tl2...>, std::tuple<Tl2...>, indices<>> {};

}
}