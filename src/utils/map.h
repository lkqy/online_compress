#pragma once
// 这不是一个完整的lzw实现，而是根据online-compress特点
// 改造的部分实现

#include <functional>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdlib.h>
#include <tbb/concurrent_priority_queue.h>
#include "utils/dictionary.h"
#include "utils/encoder.h"

/*
   将编码器、解码器抽象出来；
   字典作为一个独立容器;
   对外接口可以提供MaxMap、ExpireMap等
*/

namespace online_compress {

// 静态map
class Map {
 public:
    virtual ~Map(){};
    Map(EncoderPtr& _encoder_ptr, uint64_t _max_capacity, uint64_t _min_occrence);

    std::string compress(const std::string& data, bool enable_add_cache);

    std::string uncompress(const std::string& data);

 protected:
    virtual std::vector<uint64_t> build_index(const std::string& data);

    void update_encode(const std::string& str);

    bool add_dictionary(const std::string& key);

    bool remove_dictionary(const std::string& key);

    bool exists(const std::string& key) { return dictionary.exists_first(key); };

    uint64_t get_encode_id(const std::string& key) {
        if (key.length() == 1) {
            return key[0];
        }
        uint64_t id = 0;
        if (dictionary.get_second(key, id)) {
            return id;
        }
        assert(id > 0);
        // ToDo: 如果读不到id了，会出错
        return 0;
    };

    std::string get_decode_str(uint64_t index) {
        if (index < 256) {
            return std::string(1, index);
        }
        std::string key;
        if (dictionary.get_first(index, key)) {
            return key;
        }
        // ToDo: 如果读不到id了，会出错
        return std::string();
    };

    size_t dictionary_size() { return dictionary.size(); };

    EncoderPtr encoder_ptr;
    uint64_t max_capacity;
    uint64_t max_cache_size;
    uint64_t min_occrence;

    Dictionary<std::string, uint64_t> dictionary;

    // std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> min_heap;
    tbb::concurrent_priority_queue<uint64_t, std::greater<uint64_t>> min_heap;
};

}  // namespace online_compress
