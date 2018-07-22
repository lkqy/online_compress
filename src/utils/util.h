#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace online_compress {

template <typename T, typename H>
std::vector<H> vector_compress(std::vector<T>& vec) {
    std::vector<H> result;
    for (auto& v : vec) {
        result.push_back((H)v);
    }
    return result;
}

// 按key降序排列
template <typename T, typename H>
std::vector<std::pair<T, H>> partial_sort_unordered_map_by_key_desc(
    const std::unordered_map<T, H>& map, size_t pivot, const T& min_key) {
    typedef typename std::pair<T, H> PAIR;
    std::vector<PAIR> pairs(map.size());
    for (auto& it : map) {
        if (it.first >= min_key) {
            pairs.push_back(std::make_pair(it.first, it.second));
        }
    }
    // 局部排序
    std::partial_sort(pairs.begin(), pairs.begin() + pivot, pairs.end(),
                      [&](PAIR& i, PAIR& j) { return i.first >= j.first; });
    return pairs;
}

// 按value降序排列
template <typename T, typename H>
std::vector<std::pair<T, H>> partial_sort_unordered_map_by_value_desc(
    const std::unordered_map<T, H>& map, size_t pivot, const H& min_value) {
    typedef typename std::pair<T, H> PAIR;
    std::vector<PAIR> pairs(map.size());
    for (auto& it : map) {
        if (it.second >= min_value) {
            pairs.push_back(std::make_pair(it.first, it.second));
        }
    }
    // 局部排序
    std::partial_sort(pairs.begin(), pairs.begin() + pivot, pairs.end(),
                      [&](PAIR& i, PAIR& j) { return i.second >= j.second; });
    return pairs;
}

}  // namespace online_compress
