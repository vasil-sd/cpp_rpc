#pragma once

#include <functional>

namespace serdes {

template<typename buffer>
using writer = std::function<void(buffer&)>;
template<typename buffer>
using ser_handler = std::function<void(size_t, const writer<buffer>&)>;

template<typename format, typename buffer, typename Val, typename...T>
struct serdes {
    // to thow undefined ref to function on link stage
    // because linker message more easy to understand
    static void serialize(const Val&, const ser_handler<buffer>& handler);
    static Val deserialize(buffer&);
};

template<typename fmt, typename buffer, typename T>
buffer serialize(const T& t) {
    buffer result;
    serdes<fmt, buffer, T>::serialize(t, [&](size_t sz, const writer<buffer>& wr){
        buffer buf{sz};
        wr(buf);
        result = std::move(buf);
    });
    return std::move(result);
}

template<typename fmt, typename T, typename buffer>
T deserialize(buffer& buf) {
    return serdes<fmt, buffer, T>::deserialize(buf);
}

} // namespace serdes
