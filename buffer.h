#pragma once

#include <string.h>
#include <memory>

class buffer {
protected:
    void *data;
    size_t size;
    size_t pos;
    bool derived;
    buffer(void* d, size_t sz, bool der = false)
        : data(d)
        , size(sz)
        , pos(0)
        , derived(der)
    { }
    void reset() {
        data = nullptr;
        size = 0;
        pos = 0;
        derived = false;
    }
public:
    buffer() : data(nullptr) { }

    buffer(buffer&& b) : data(b.data), size(b.size), pos(b.pos), derived(b.derived)
    {
        b.data = nullptr;
        b.reset();
    }

    buffer& operator=(buffer&) = delete;
    buffer(const buffer&) = delete;

    buffer& operator=(buffer&& b) {
        if (!derived && data != nullptr)
            free(data);
        data = b.data;
        size = b.size;
        pos = b.pos;
        derived = b.derived;
        b.reset();
        return *this;
    }

    buffer(size_t sz) : buffer(malloc(sz), sz) { }

    ~buffer() {
        if (!derived && data != nullptr)
            free(data);
        reset();
    }

    buffer limit(size_t sz) {
        // assert sz <= size - pos
        size_t sub_pos = pos;
        pos += sz;
        return {static_cast<char*>(data) + sub_pos, sz, true};
    }

    size_t length() {
        return size;
    }

    size_t space() {
        return size - pos;
    }

    void rewind() {
        pos = 0;
    }
};

class wbuffer : public buffer {
public:
    wbuffer(size_t sz) : buffer(sz) { }
    wbuffer() = default;
    wbuffer(buffer&& b) : buffer(std::move(b)) {};
    wbuffer(wbuffer&&) = default;
    wbuffer& operator=(wbuffer&& wb) {
        buffer::operator=(std::move(wb));
        return *this;
    }
    void write(const void* d, size_t sz) {
        // assert sz <= size - pos
        memcpy(static_cast<char*>(data) + pos, d, sz);
        pos += sz;
    }
    template<typename T>
    void write(const T& t) {
        write(reinterpret_cast<void *>(&const_cast<T&>(t)), sizeof(T));
    }
    wbuffer limit(size_t sz) {
        return {buffer::limit(sz)};
    }
};

class rbuffer : public buffer {
public:
    rbuffer() = default;
    rbuffer(buffer&& b) : buffer(std::move(b)) { rewind(); };
    rbuffer(rbuffer&&) = default;
    rbuffer& operator=(rbuffer&& rb) {
        buffer::operator=(std::move(rb));
        return *this;
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
    rbuffer limit(size_t sz) {
        return {buffer::limit(sz)};
    }
    void rewind() { pos = 0; }
};
