#pragma once

#include "buffer.h"
#include "meta.h"
#include <functional>
#include <tuple>

template<typename wbuffer>
using writer = std::function<void(wbuffer&)>;
template<typename wbuffer>
using ser_handler = std::function<void(size_t, writer<wbuffer>)>;

template<typename rbuffer, typename T>
using reader = std::function<T(rbuffer&)>;
template<typename rbuffer, typename T>
using deser_handler = std::function<T(size_t, reader<rbuffer, T>)>;


template<typename format, typename wbuffer, typename rbuffer, typename Val, typename...T>
struct serdes {
    // to thow undefined ref to function on link stage
    // because linker message more easy to understand
    static void serialize(Val&, ser_handler<wbuffer>& handler);
    static Val deserialize(rbuffer&);
};

// ====== types of format ======

struct simple{};
struct msgpack{};

// ====== primitive types =======

//* bool
template<typename wbuffer, typename rbuffer>
struct serdes<simple, wbuffer, rbuffer, bool> {
    static void serialize(bool& val, ser_handler<wbuffer> handler) {
        handler(sizeof(bool), [&](wbuffer& buf){
            buf.write(val);
        });
    }
    static bool deserialize(rbuffer& buf) {
        bool result;
        buf.read(result);
        return result;
    }
};

//* int

template<typename wbuffer, typename rbuffer>
struct serdes<simple, wbuffer, rbuffer, int> {
    static void serialize(int& val, ser_handler<wbuffer> handler) {
        handler(sizeof(int), [&](wbuffer& buf){
            buf.write(val);
        });
    }
    static int deserialize(rbuffer& buf) {
        int result;
        buf.read(result);
        return result;
    }
};

//* std::string

template<typename wbuffer, typename rbuffer>
struct serdes<simple, wbuffer, rbuffer,std::string> {
    static void serialize(std::string& val, ser_handler<wbuffer> handler) {
        handler(val.size() + sizeof(size_t), [&](wbuffer& buf){
            size_t s = val.size();
            buf.write(s);
            buf.write(val.data(), s);
        });
    };
    static std::string deserialize(rbuffer& buf) {
        size_t s;
        buf.read(s);
        void *d = malloc(s);
        buf.read(d, s);
        std::string str{reinterpret_cast<char*>(d), s};
        free(d);
        return str;
    }
};

// ====== Containers ======


// --------- Tuple -----------
template<typename...> struct tl;

template<typename format, typename wbuffer, typename rbuffer, typename Arg, typename...TupleArgs, typename...Rest, typename...Prev>
struct serdes<format, wbuffer, rbuffer, std::tuple<TupleArgs...>, tl<Arg, Rest...>, tl<Prev...>> :
       serdes<format, wbuffer, rbuffer,std::tuple<TupleArgs...>, tl<Rest...>, tl<Prev..., Arg>> {
    using next = serdes<format, wbuffer, rbuffer, std::tuple<TupleArgs...>, tl<Rest...>, tl<Prev..., Arg>>;
    template<int N>
    static void serialize_(std::tuple<TupleArgs...>& tuple, size_t sz, ser_handler<wbuffer>& handler, writer<wbuffer> process) {
        serdes<format, wbuffer, rbuffer, Arg>::serialize(std::get<N>(tuple), [&](size_t new_sz, writer<wbuffer> fn){
            next::template serialize_<N+1>(tuple, sz + new_sz, handler, [&](wbuffer& buf){
                process(buf);
                auto b = buf.limit(new_sz);
                fn(b);
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
    static void serialize_(std::tuple<TupleArgs...>&, size_t sz, ser_handler<wbuffer>& handler, writer<wbuffer> process) {
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
    static void serialize(std::tuple<TupleArgs...>& tuple, ser_handler<wbuffer> handler) {
        next::template serialize_<0>(tuple, 0, handler, [](wbuffer&){});
    }
    static std::tuple<TupleArgs...> deserialize(rbuffer& buf) {
        return next::deserialize(buf);
    }
};

// ====== for POD-like structures ======

template<typename format, typename wbuffer, typename rbuffer, class S>
struct struct_serdes {
    using tuple_type = decltype(to_tuple(S{}));
    using sd = serdes<format, wbuffer, rbuffer, tuple_type>;
    static void serialize(S& val, ser_handler<wbuffer> handler) {
        auto tup = to_tuple(val);
        sd::serialize(tup, handler);
    };
    static S deserialize(rbuffer& buf) {
        auto tup = sd::deserialize(buf);
        return from_tuple<S>(tup);
    }
};

#define STRUCT_SERDES(S) \
template<typename format, typename wbuffer, typename rbuffer> \
struct serdes<format, wbuffer, rbuffer, S> : struct_serdes<format, wbuffer, rbuffer, S> {};

// ====== simple functions ======

template<typename wbuffer, typename T, typename Fmt = simple>
wbuffer serialize(T& t) {
    wbuffer result;
    serdes<Fmt, wbuffer, rbuffer, T>::serialize(t, [&](size_t sz, writer<wbuffer> wr){
        wbuffer buf{sz};
        wr(buf);
        result = std::move(buf);
    });
    return std::move(result);
}

template<typename T, typename rbuffer, typename Fmt = simple>
T deserialize(rbuffer& buf) {
    return serdes<Fmt, wbuffer, rbuffer, T>::deserialize(buf);
}
