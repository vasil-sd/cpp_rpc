#pragma once

#include "serdes/serdes.h"
#include <tuple>

namespace serdes {

namespace {
template<typename...> struct tl;
} // anonymous namespace

template<typename format, typename buffer, typename Arg, typename...TupleArgs, typename...Rest, typename...Prev>
struct serdes<format, buffer, std::tuple<TupleArgs...>, tl<Arg, Rest...>, tl<Prev...>> :
       serdes<format, buffer, std::tuple<TupleArgs...>, tl<Rest...>, tl<Prev..., Arg>> {
    using next = serdes<format, buffer, std::tuple<TupleArgs...>, tl<Rest...>, tl<Prev..., Arg>>;
    template<int N>
    static void serialize_(const std::tuple<TupleArgs...>& tuple, size_t sz, const ser_handler<buffer>& handler, const writer<buffer>& process) {
        serdes<format, buffer, Arg>::serialize(std::get<N>(tuple), [&](size_t new_sz, const writer<buffer>& fn){
            next::template serialize_<N+1>(tuple, sz + new_sz, handler, [&](buffer& buf){
                process(buf);
                fn(buf);
            });
        });
    }
    static std::tuple<TupleArgs...> deserialize(buffer& buf, Prev&&...args) {
        return next::deserialize(buf, std::move(args)..., serdes<format, buffer, Arg>::deserialize(buf));
    }
};

template<typename format, typename buffer, typename...TupleArgs>
struct serdes<format, buffer, std::tuple<TupleArgs...>, tl<>, tl<TupleArgs...>> {
    template<int>
    static void serialize_(const std::tuple<TupleArgs...>&, size_t sz, const ser_handler<buffer>& handler, const writer<buffer>& process) {
        handler(sz, [&](buffer& buf){
            process(buf);
        });
    }
    static std::tuple<TupleArgs...> deserialize(buffer& buf, TupleArgs&&...args) {
        return {std::move(args)...};
    }
};

template<typename format, typename buffer, typename...TupleArgs>
struct serdes<format, buffer, std::tuple<TupleArgs...>> :
       serdes<format, buffer, std::tuple<TupleArgs...>, tl<TupleArgs...>, tl<>> {
    using next = serdes<format, buffer, std::tuple<TupleArgs...>, tl<TupleArgs...>, tl<>>;
    static void serialize(const std::tuple<TupleArgs...>& tuple, const ser_handler<buffer>& handler) {
        next::template serialize_<0>(tuple, 0, handler, [](buffer&){});
    }
    static std::tuple<TupleArgs...> deserialize(buffer& buf) {
        return next::deserialize(buf);
    }
};

} // namespace serdes
