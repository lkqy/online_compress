#pragma once

#include <stdlib.h>

namespace online_compress {

uint32_t MurmurHash(char* buff, int len);

template <typename T>
struct MurmurHashCompare {
    static size_t hash(const T& x) { return MurmurHash((char*)(&x), sizeof(T)); };
    static bool equal(const T& a, const T& b) { return a == b; };
};

template <typename T>
struct MurmurHashAtomicCompare {
    typedef typename T::value_type TYPE;
    static size_t hash(const T& x) {
        TYPE t = x.load();
        return MurmurHash((char*)(&x), sizeof(TYPE));
    };
    static bool equal(const T& a, const T& b) {
        TYPE _a = a.load();
        TYPE _b = b.load();
        return _a == _b;
    };
};

}  // namespace online_compress
