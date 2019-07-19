#pragma once

#include <functional>

namespace serdes {

template<typename wbuffer>
using writer = std::function<void(wbuffer&)>;
template<typename wbuffer>
using ser_handler = std::function<void(size_t, const writer<wbuffer>&)>;

template<typename format, typename wbuffer, typename rbuffer, typename Val, typename...T>
struct serdes {
    // to thow undefined ref to function on link stage
    // because linker message more easy to understand
    static void serialize(const Val&, const ser_handler<wbuffer>& handler);
    static Val deserialize(rbuffer&);
};

template<typename fmt, typename wbuffer, typename T>
wbuffer serialize(const T& t) {
    wbuffer result;
    serdes<fmt, wbuffer, wbuffer, T>::serialize(t, [&](size_t sz, const writer<wbuffer>& wr){
        wbuffer buf{sz};
        wr(buf);
        result = std::move(buf);
    });
    return std::move(result);
}

template<typename fmt, typename T, typename rbuffer>
T deserialize(rbuffer& buf) {
    return serdes<fmt, rbuffer, rbuffer, T>::deserialize(buf);
}

} // namespace serdes
