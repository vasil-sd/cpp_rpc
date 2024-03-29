#pragma once

#include <type_traits>
#include <tuple>
#include "hash/hash.h"

namespace {

class method_info {

    template<int ...>
    struct indices { };

    template<int N, int ...S>
    struct gen_indices : gen_indices<N-1, N-1, S...> { };

    template<int ...S>
    struct gen_indices<0, S...> {
        typedef indices<S...> type;
    };

public:

    template <typename R, typename Class, typename... T>
    static std::tuple<typename std::decay<T>::type...>
        method_args_as_tuple(R (Class::*)(T...));

    template <typename R, typename Class, typename... T>
    static R method_result(R (Class::*)(T...));

    template <typename R, typename Class, typename... T>
    static Class method_class(R (Class::*)(T...));

    template<typename C, typename L, typename ...Args>
    static auto call_method(C& c, L l, std::tuple<Args...> args) -> decltype(method_result(l)) {
       return method_info::call_method_helper(c, l, args, typename gen_indices<sizeof...(Args)>::type());
    }

private:
    template<typename C, typename L, typename A, int ...S>
    static auto call_method_helper(C& c, L l, A&& a, indices<S...>) -> decltype(method_result(l)) {
       return (c.*l)(std::move(std::get<S>(a))...);
    }
};

} // anonymous namespace

template<auto member>
struct method : public method_info {

    using class_type = decltype(method_class(member));
    using result_type = decltype(method_result(member));    
    using args_type = decltype(method_args_as_tuple(member));
    constexpr static hash::UI64 signature_hash = hash::type_hash<std::tuple<class_type, result_type, args_type>>::value;

    static result_type call(class_type& obj, args_type&& args) {
        return call_method(obj, member, std::move(args));
    }
};
