#pragma once

#include "serdes/serdes.h"
#include <tuple>

namespace serdes {

namespace {
template<typename...> struct tl;
} // anonymous namespace

template<typename format, typename wbuffer, typename rbuffer, typename Arg, typename...TupleArgs, typename...Rest, typename...Prev>
struct serdes<format, wbuffer, rbuffer, std::tuple<TupleArgs...>, tl<Arg, Rest...>, tl<Prev...>> :
       serdes<format, wbuffer, rbuffer,std::tuple<TupleArgs...>, tl<Rest...>, tl<Prev..., Arg>> {
    using next = serdes<format, wbuffer, rbuffer, std::tuple<TupleArgs...>, tl<Rest...>, tl<Prev..., Arg>>;
    template<int N>
    static void serialize_(const std::tuple<TupleArgs...>& tuple, size_t sz, const ser_handler<wbuffer>& handler, const writer<wbuffer>& process) {
        serdes<format, wbuffer, rbuffer, Arg>::serialize(std::get<N>(tuple), [&](size_t new_sz, const writer<wbuffer>& fn){
            next::template serialize_<N+1>(tuple, sz + new_sz, handler, [&](wbuffer& buf){
                process(buf);
                fn(buf);
            });
        });
    }
    static std::tuple<TupleArgs...> deserialize(rbuffer& buf, Prev&&...args) {
        return next::deserialize(buf, std::move(args)..., serdes<format, wbuffer, rbuffer, Arg>::deserialize(buf));
    }
};

template<typename format, typename wbuffer, typename rbuffer, typename...TupleArgs>
struct serdes<format, wbuffer, rbuffer, std::tuple<TupleArgs...>, tl<>, tl<TupleArgs...>> {
    template<int>
    static void serialize_(const std::tuple<TupleArgs...>&, size_t sz, const ser_handler<wbuffer>& handler, const writer<wbuffer>& process) {
        handler(sz, [&](wbuffer& buf){
            process(buf);
        });
    }
    static std::tuple<TupleArgs...> deserialize(rbuffer& buf, TupleArgs&&...args) {
        return {std::move(args)...};
    }
};

template<typename format, typename wbuffer, typename rbuffer, typename...TupleArgs>
struct serdes<format, wbuffer, rbuffer, std::tuple<TupleArgs...>> :
       serdes<format, wbuffer, rbuffer,std::tuple<TupleArgs...>, tl<TupleArgs...>, tl<>> {
    using next = serdes<format, wbuffer, rbuffer, std::tuple<TupleArgs...>, tl<TupleArgs...>, tl<>>;
    static void serialize(const std::tuple<TupleArgs...>& tuple, const ser_handler<wbuffer>& handler) {
        next::template serialize_<0>(tuple, 0, handler, [](wbuffer&){});
    }
    static std::tuple<TupleArgs...> deserialize(rbuffer& buf) {
        return next::deserialize(buf);
    }
};

} // namespace serdes
