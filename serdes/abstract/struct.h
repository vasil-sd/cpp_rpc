#pragma once

#include "tuple.h"
#include "meta/tuple.h"

namespace serdes {

template<typename format, typename buffer, class S>
struct struct_serdes {
    using tuple_type = decltype(meta::tuple::to_tuple(S{}));
    using sd = serdes<format, buffer, tuple_type>;
    static void serialize(const S& val, const ser_handler<buffer>& handler) {
        auto tup = meta::tuple::to_tuple(val);
        sd::serialize(tup, handler);
    };
    static S deserialize(buffer& buf) {
        return meta::tuple::from_tuple<S>(sd::deserialize(buf));
    }
};

#define STRUCT_SERDES(S) \
namespace serdes { \
template<typename format, typename buffer> \
struct serdes<format, buffer, S> : struct_serdes<format, buffer, S> {}; \
}

} // namespace serdes
