#pragma once

#include "serdes/serdes.h"

namespace serdes {

// ====== types of format ======

namespace format {

struct simple{};

}

// ====== primitive types =======

//* bool
template<typename buffer>
struct serdes<format::simple, buffer, bool> {
    static void serialize(bool& val, ser_handler<buffer> handler) {
        handler(sizeof(bool), [&](buffer& buf){
            buf.write(val);
        });
    }
    static bool deserialize(buffer& buf) {
        bool result;
        buf.read(result);
        return result;
    }
};

//* int

template<typename buffer>
struct serdes<format::simple, buffer, int> {
    static void serialize(int& val, ser_handler<buffer> handler) {
        handler(sizeof(int), [&](buffer& buf){
            buf.write(val);
        });
    }
    static int deserialize(buffer& buf) {
        int result;
        buf.read(result);
        return result;
    }
};

//* std::string

template<typename buffer>
struct serdes<format::simple, buffer, std::string> {
    static void serialize(std::string& val, ser_handler<buffer> handler) {
        handler(val.size() + sizeof(size_t), [&](buffer& buf){
            size_t s = val.size();
            buf.write(s);
            buf.write(val.data(), s);
        });
    };
    static std::string deserialize(buffer& buf) {
        size_t s;
        buf.read(s);
        void *d = malloc(s);
        buf.read(d, s);
        std::string str{reinterpret_cast<char*>(d), s};
        free(d);
        return str;
    }
};

} // namespace serdes
