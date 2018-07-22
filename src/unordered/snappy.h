#pragma once

#include <algorithm>
#include <atomic>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <snappy.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>

#include "utils/hash.h"

namespace online_compress {

namespace unordered {

template <typename H>
std::string compress_helper(std::vector<H>& vec, bool log = false) {
    if (vec.size() == 0) {
        return std::string();
    }
    // 先排序，再压缩(降序)
    std::sort(vec.begin(), vec.end());
    if (log) {
        for (auto v : vec) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }
    char* input = new char[vec.size() * sizeof(H)];
    H* f = reinterpret_cast<H*>(input);
    for (auto v : vec) {
        *f++ = v;
    }
    int length = reinterpret_cast<char*>(f) - input;
    std::string output;
    snappy::Compress(input, length, &output);
    return output;
}

template <typename H>
std::vector<H> uncompress_helper(std::string& data) {
    std::vector<H> result;
    if (data.size()) {
        return result;
    }
    std::string _output;
    snappy::Uncompress(data.c_str(), data.size(), &_output);
    const H* s = reinterpret_cast<const H*>(_output.c_str());
    const H* e = reinterpret_cast<const H*>(_output.c_str() + _output.size());
    while (s < e) {
        result.push_back(*s++);
    }
    return result;
}

//无序vector压缩
template <typename T>
class VectorCompress {
 public:
    typedef typename tbb::concurrent_hash_map<uint32_t, T, MurmurHashCompare<uint32_t>> MAP_CACHE;
    typedef typename tbb::concurrent_hash_map<T, uint32_t, MurmurHashCompare<T>> MAP_RCACHE;
    // 换成线程安全的计数器, atomic 没有拷贝函数，好奇怪
    // typedef typename tbb::concurrent_hash_map<T, std::atomic<int>> MAP_COUNT;
    typedef typename tbb::concurrent_hash_map<T, int> MAP_COUNT;
    typedef typename tbb::concurrent_vector<T> VEC_QUEUE;
    VectorCompress(size_t max_lookup = 1000000, size_t mini_currence = 3, float ratio = 0.95)
        : _max_lookup(max_lookup), _ratio(ratio), _mini_currence(mini_currence), _map_index(0){};

    std::string compress(const std::vector<T>& vec, bool log = false) {
        if (vec.size() == 0) {
            return std::string();
        }
        std::vector<size_t> left;
        std::vector<T> right;
        for (const auto& v : vec) {
            size_t i;
            if (get(v, i)) {
                add_ref(v);
                left.push_back(i);
                if (log) {
                    std::cout << i << " ";
                }
            } else {
                right.push_back(v);
            }
            add_lookup(v);
        }
        if (log) std::cout << "\n";
        std::string left_str = compress_helper<size_t>(left, log);
        std::string right_str = compress_helper<T>(right);
        char input[4];
        uint32_t size = left_str.size();
        char* f = reinterpret_cast<char*>(&size);
        input[0] = f[0];
        input[1] = f[1];
        input[2] = f[2];
        input[3] = f[3];
        return std::string(input, 4) + left_str + right_str;
    };
    std::vector<T> uncompress(const std::string& data) {
        if (data.empty()) {
            return std::vector<T>();
        }
        char input[4];
        const char* _data = data.c_str();
        input[0] = _data[0];
        input[1] = _data[1];
        input[2] = _data[2];
        input[3] = _data[3];
        uint32_t* size = reinterpret_cast<uint32_t*>(input);
        std::string left_str = data.substr(4, *size);
        std::string right_str = data.substr(4 + *size);
        std::vector<size_t> left_index = uncompress_helper<size_t>(left_str);
        std::vector<T> left;
        for (size_t v : left_index) {
            T t = remove(v);
            left.push_back(t);
        }
        std::vector<T> right = uncompress_helper<T>(right_str);
        left.insert(left.end(), right.begin(), right.end());
        return left;
    };

 private:
    bool get(const T& t, size_t& i) {
        typename MAP_RCACHE::accessor a;
        if (reverse_cache.find(a, t)) {
            i = a->second;
            return true;
        }
        return false;
    };
    bool add_ref(const T& t) {
        typename MAP_COUNT::accessor a;
        if (key_cnt.find(a, t)) {
            a->second += 1;
            return true;
        }
        return false;
    };
    T remove(const size_t t) {
        typename MAP_CACHE::accessor a;
        if (cache.find(a, t)) {
            // 实时清理cache
            clear_cache(t);
        }
        return t;
    };
    bool add_lookup(const T& t) {
        lookup.push_back(t);
        if (lookup.size() <= _max_lookup) {
            return true;
        }
        // 必须加锁，防止其他线程进入, 使用
        std::lock_guard<std::mutex> clear_lock(clear_mtx);  //稍后修改成更好的锁
        std::lock_guard<std::mutex> add_lock(add_mtx);      //稍后修改成更好的锁
        if (true) {
            std::cout << "add compress support\n";
            // 更新cache
            std::unordered_map<T, size_t> lookup_currence;
            for (auto& t : lookup) {
                lookup_currence[t] += 1;
            }
            // typedef typename std::unordered_map<T, size_t>::iterator pair;
            typedef typename std::pair<T, size_t> PAIR;
            std::vector<PAIR> pairs(lookup_currence.size());
            for (auto& it : lookup_currence) {
                // 过滤掉频次太低的数据
                if (it.second >= _mini_currence) {
                    pairs.push_back(std::make_pair(it.first, it.second));
                }
            }
            const size_t pivot = size_t(_ratio * pairs.size());
            if (pairs.size() == 0 || pivot == 0) {
                // 尽早返回
                lookup.clear();  // 防止某一段时间内全都是稀疏数，导致无法有效压缩
                return true;
            }
            // 局部排序
            std::partial_sort(pairs.begin(), pairs.begin() + pivot, pairs.end(),
                              [&](PAIR& i, PAIR& j) { return i.second >= j.second; });
            int ret = true;
            for (size_t i = 0; i < pivot; ++i) {
                auto& pair = pairs[i];
                size_t _i;
                if (!get(pair.first, _i)) {
                    size_t size = _map_index.fetch_add(1, std::memory_order_relaxed);
                    typename MAP_RCACHE::accessor a0;
                    typename MAP_CACHE::accessor a1;
                    // 两个必须同时成功，否则不插入
                    bool r0 = reverse_cache.insert(a0, pair.first);
                    bool r1 = cache.insert(a1, size);
                    if (r0 && r1) {
                        a0->second = size;
                        a1->second = pair.first;
                    } else {
                        // 如果插入失败两个都再删除
                        ret = false;
                        if (r0) {
                            reverse_cache.erase(a0);
                        } else if (r1) {
                            cache.erase(a1);
                        }
                    }
                }
            }
            // 完成lookup后，就清除数据
            lookup.clear();
            return ret;
        }
        return true;
    };
    bool clear_cache(const T& t) {
        // ToDo: 加锁
        typename MAP_COUNT::accessor a;
        if (!key_cnt.find(a, t)) {
            return false;
        }
        //得看下怎么优化锁
        std::lock_guard<std::mutex> lock(clear_mtx);  //稍后修改成更好的锁
        if (!key_cnt.find(a, t)) {
            return false;
        }
        a->second -= 1;
        // 这里的锁不好搞，后面再优化
        if (a->second == 0) {
            typename MAP_RCACHE::accessor a1;
            if (reverse_cache.find(a1, t)) {
                size_t i = a1->second;
                cache.erase(i);
                reverse_cache.erase(t);
                key_cnt.erase(t);
            }
            return true;
        }
        return false;
    };
    MAP_CACHE cache;
    MAP_RCACHE reverse_cache;
    MAP_COUNT key_cnt;

    VEC_QUEUE lookup;
    const size_t _max_lookup;
    const float _ratio;
    const size_t _mini_currence;
    std::atomic<size_t> _map_index;
    std::mutex add_mtx;
    std::mutex clear_mtx;
};

}  // namespace unordered
}  // namespace online_compress
