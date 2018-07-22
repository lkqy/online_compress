#pragma once

#include <unordered_map>
#include <assert.h>

namespace online_compress {

template <typename T, typename H>
class Dictionary {
 public:
    size_t size() {
        assert(h2t.size() == t2h.size());
        return h2t.size();
    };
    bool exists_first(const T& t) { return t2h.find(t) != t2h.end(); };

    bool exists_second(const H& h) { return h2t.find(h) != h2t.end(); };

    H get_second(const T& t) {
        assert(exists_second(t));
        return t2h[t];
    };

    T get_first(const H& h) {
        assert(exists_first(h));
        return h2t[h];
    };

    bool erase_first(const T& t) {
        assert(exists_second(t));
        H h = get_second(t);
        h2t.erase(h);
        t2h.erase(t);
        return true;
    };
    bool erase_second(const H& h) {
        assert(exists_first(h));
        T t = get_first(h);
        h2t.erase(h);
        t2h.erase(t);
        return true;
    };

    bool exists(const T& t, const H& h) { return exists_first(t) && exists_second(h); };
    bool add(const T& t, const H& h) {
        // 先这么判断着，实际上需要关心的是t <-> h不能破坏原先的1-1关系
        assert(!exists_first[t] && !exists_second(h));
        t2h[t] = h;
        h2t[h] = t;
        return true;
    };
    bool erase(const T& t, const H& h) {
        assert(exists_first[t] && exists_second(h));
        t2h.erase(t);
        h2t.erase(h);
        return true;
    };

 private:
    std::unordered_map<T, H> t2h;
    std::unordered_map<H, T> h2t;
};

}  // namespace online_compress
