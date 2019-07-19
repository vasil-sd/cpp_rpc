#pragma once

#include "serdes/serdes.h"

namespace serdes {

// ====== types of format ======

namespace format {

struct simple{};

}

// ====== primitive types =======

//* bool
template<typename wbuffer, typename rbuffer>
struct serdes<format::simple, wbuffer, rbuffer, bool> {
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
struct serdes<format::simple, wbuffer, rbuffer, int> {
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
struct serdes<format::simple, wbuffer, rbuffer,std::string> {
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

} // namespace serdes
