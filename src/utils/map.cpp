#include "utils/map.h"
#include <iostream>
#include <snappy.h>
#include "utils/util.h"

namespace online_compress {

// 过滤掉出现频次太低的，按权重（str_size * weight) 取topk
std::vector<std::string> get_top_usefull_string(std::unordered_map<std::string, uint64_t>& map,
                                                size_t min_occrence, size_t topk,
                                                bool weigth_enable) {
    typedef typename std::pair<std::string, uint64_t> PAIR;
    std::vector<PAIR> vec;
    for (auto& it : map) {
        if (it.second >= min_occrence) {
            uint64_t w = 1;
            if (weigth_enable) {
                w = it.first.length();
            }
            vec.push_back(std::make_pair(it.first, w * it.second));
        }
    }
    // 局部排序
    std::partial_sort(vec.begin(), vec.begin() + topk, vec.end(),
                      [&](PAIR& i, PAIR& j) { return i.second >= j.second; });

    std::vector<std::string> result;
    for (size_t i = 0; i < topk; ++i) {
        result.push_back(vec[i].first);
    }
    return result;
}

Map::Map(EncoderPtr& _encoder_ptr, uint64_t _max_capacity, uint64_t _min_occrence)
    : encoder_ptr(_encoder_ptr), max_capacity(_max_capacity), min_occrence(_min_occrence) {
    // 从256开始
    for (uint64_t i = 256; i < max_capacity; ++i) {
        min_heap.push(i);
    }
}

std::string Map::compress(const std::string& data, bool enable_add_cache) {
    if (data.empty()) {
        return std::string();
    }
    if (enable_add_cache) {
        update_encode(data);
    }
    std::vector<uint64_t> result = build_index(data);
    //压缩成4字节
    std::vector<uint32_t> compress = vector_compress<uint64_t, uint32_t>(result);
    const size_t length = result.size() * sizeof(uint32_t);
    const char* p = reinterpret_cast<const char*>(compress.data());
    return std::string(p, length);
}

std::vector<uint64_t> Map::build_index(const std::string& data) {
    std::vector<uint64_t> result;
    const size_t size = data.length();
    size_t i = 0;
    std::string pre_str;
    // 贪心匹配最大字符串，找到后记录(这里可能是优化点)
    while (i < size) {
        std::ostringstream oss;
        oss << data[i];
        pre_str = oss.str();
        ++i;
        while (i < size) {
            oss << data[i];
            std::string cur_str = oss.str();
            if (exists(cur_str)) {
                pre_str = cur_str;
                ++i;
            } else {
                result.push_back(get_encode_id(pre_str));
                break;
            }
        }
    }
    if (pre_str.length() > 0) {
        result.push_back(get_encode_id(pre_str));
    }
    return result;
}

std::string Map::uncompress(const std::string& data) {
    if (data.empty()) {
        return std::string();
    }
    // 解压
    //强制转换成uint64_t 数字
    const uint32_t* p = reinterpret_cast<const uint32_t*>(data.c_str());
    const size_t size = data.size() / sizeof(uint32_t);

    // 查表输出
    std::ostringstream oss;
    for (size_t i = 0; i < size; ++i) {
        oss << get_decode_str(p[i]);
    }
    return oss.str();
}

void Map::update_encode(const std::string& str) {
    std::unordered_map<std::string, uint64_t> map;
    if (encoder_ptr->add(str, map)) {
        // 更新到map cache里
        if (map.size() + dictionary_size() < max_capacity) {
            for (auto& it : map) {
                add_dictionary(it.first);
            }
        } else {
            size_t pivot = max_capacity - dictionary_size();
            auto vec = get_top_usefull_string(map, min_occrence, pivot, false);
            // 取最高的map 内容到
            for (auto& v : vec) {
                add_dictionary(v);
            }
        }
    }
}

bool Map::add_dictionary(const std::string& key) {
    //添加到字典里面
    // 要找一个有效id存进去
    if (min_heap.empty() || exists(key)) {
        return false;
    }

    uint64_t id = 0;
    if (min_heap.try_pop(id)) {
        if (dictionary.add(key, id)) {
            return true;
        } else {
            min_heap.push(id);
        }
    }
    return false;
}

bool Map::remove_dictionary(const std::string& key) {
    if (!exists(key)) {
        return false;
    }
    uint64_t id = 0;
    if (dictionary.get_second(key, id)) {
        if (dictionary.erase_first(key)) {
            min_heap.push(id);
            return true;
        }
    }
    return false;
}

}  // namespace online_compress
