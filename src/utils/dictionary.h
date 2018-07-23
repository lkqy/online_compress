#pragma once

#include <unordered_map>
#include <assert.h>
#include <tbb/concurrent_hash_map.h>

namespace online_compress {

// add、remove状态下线程不安全
template <typename T, typename H>
class Dictionary {
 public:
    size_t size() {
        assert(h2t.size() == t2h.size());
        return h2t.size();
    };
    bool exists_first(const T& t) { return t2h.count(t) > 0; };

    bool exists_second(const H& h) { return h2t.count(h) > 0; };

    bool get_second(const T& t, H& h) {
        typename tbb::concurrent_hash_map<T, H>::accessor a;
        if (t2h.find(a, t)) {
            h = a->second;
            return true;
        }
        return false;
    };

    bool get_first(const H& h, T& t) {
        typename tbb::concurrent_hash_map<H, T>::accessor a;
        if (h2t.find(a, h)) {
            t = a->second;
            return true;
        }
        return false;
    };

    bool erase_first(const T& t) {
        // 尽力交付
        H h;
        if (get_second(t, h)) {
            h2t.erase(h);
        }
        return t2h.erase(t);
    };
    bool erase_second(const H& h) {
        // 尽力交付
        T t;
        if (get_first(h, t)) {
            t2h.erase(t);
        }
        return h2t.erase(h);
    };

    bool exists(const T& t, const H& h) { return exists_first(t) && exists_second(h); };
    bool add(const T& t, const H& h) {
        // 先这么判断着，实际上需要关心的是t <-> h不能破坏原先的1-1关系
        assert(!exists_first[t] && !exists_second(h));
        typename tbb::concurrent_hash_map<T, H>::accessor a_t;
        typename tbb::concurrent_hash_map<H, T>::accessor a_h;
        bool f_t = t2h.insert(a_t, t);
        bool f_h = h2t.insert(a_h, h);
        if (f_t && f_h) {
            a_t->second = h;
            a_h->second = t;
            return exists(t, h);
        } else if (f_t) {
            t2h.erase(a_t);
        } else if (f_h) {
            h2t.erase(a_h);
        }
        return false;
    };
    bool erase(const T& t, const H& h) {
        assert(exists_first[t] && exists_second(h));
        return t2h.erase(t) && h2t.erase(h);
    };

 private:
    tbb::concurrent_hash_map<T, H> t2h;
    tbb::concurrent_hash_map<H, T> h2t;
};

}  // namespace online_compress
