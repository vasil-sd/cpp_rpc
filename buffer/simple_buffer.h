#pragma once

#include <string.h>
#include <memory>

class buffer {
private:
    void *data;
    size_t size;
    size_t pos;
    buffer(void* d, size_t sz)
        : data(d)
        , size(sz)
        , pos(0)
    { }
    void reset() {
        data = nullptr;
        size = 0;
        pos = 0;
    }
public:
    buffer() : data(nullptr) { }

    buffer(buffer&& b) : data(b.data), size(b.size), pos(b.pos)
    {
        b.data = nullptr;
        b.reset();
    }

    buffer& operator=(buffer&) = delete;
    buffer(const buffer&) = delete;

    buffer& operator=(buffer&& b) {
        if (data != nullptr)
            free(data);
        data = b.data;
        size = b.size;
        pos = b.pos;
        b.reset();
        return *this;
    }

    buffer(size_t sz) : buffer(malloc(sz), sz) { }

    ~buffer() {
        if (data != nullptr)
            free(data);
        reset();
    }

    size_t length() { return size; }

    size_t space() { return size - pos; }

    void rewind() { pos = 0; }

    void write(const void* d, size_t sz) {
        // assert sz <= size - pos
        memcpy(static_cast<char*>(data) + pos, d, sz);
        pos += sz;
    }

    template<typename T>
    void write(const T& t) {
        write(reinterpret_cast<void *>(&const_cast<T&>(t)), sizeof(T));
    }

    void read(void* d, size_t sz) {
        // assert sz <= size - pos
        memcpy(d, static_cast<char*>(data) + pos, sz);
        pos += sz;
    }

    template<typename T>
    void read(T& t) {
        read(reinterpret_cast<void *>(&t), sizeof(T));
    }
};
