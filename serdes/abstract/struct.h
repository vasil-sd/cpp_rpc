#pragma once

#include "tuple.h"
#include "meta/tuple.h"

namespace serdes {

template<typename format, typename wbuffer, typename rbuffer, class S>
struct struct_serdes {
    using tuple_type = decltype(meta::tuple::to_tuple(S{}));
    using sd = serdes<format, wbuffer, rbuffer, tuple_type>;
    static void serialize(const S& val, const ser_handler<wbuffer>& handler) {
        auto tup = meta::tuple::to_tuple(val);
        sd::serialize(tup, handler);
    };
    static S deserialize(rbuffer& buf) {
        return meta::tuple::from_tuple<S>(sd::deserialize(buf));
    }
};

#define STRUCT_SERDES(S) \
namespace serdes { \
template<typename format, typename wbuffer, typename rbuffer> \
struct serdes<format, wbuffer, rbuffer, S> : struct_serdes<format, wbuffer, rbuffer, S> {}; \
}

} // namespace serdes
