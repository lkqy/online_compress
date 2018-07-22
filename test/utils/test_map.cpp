#include <iostream> 
#include <memory> 
#include <algorithm> 
#include <stdlib.h> 
#include <string>
#include <vector>
#include <snappy.h>
#include <gtest/gtest.h>
#include "utils/encoder.h"
#include "utils/map.h"


TEST(OnlineCompressMap, TestMapNormalUse) {
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwEncoder>();;
    online_compress::Map map(coder_ptr, 1000000, 100, 2);
    std::string data("abcabcabcabcabcabc");
    std::string less_data = map.compress(data, false);
    std::string origin_data = map.uncompress(less_data);
    ASSERT_EQ(data, origin_data);
    std::cout<<"raw:"<<data.size()<<", compress:"<<less_data.size()<<"\n";
}

TEST(OnlineCompressMap, TestMapCache) {
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwEncoder>();;
    int cache_size = 2000;
    online_compress::Map map(coder_ptr, 1000000, cache_size*2, 2);
    for (int i = 0; i < cache_size*2; ++i) {
        std::string data("abcabcabcabcabcabc");
        std::string less_data = map.compress(data, true);
    }
    for (int i = 0; i < cache_size*2; ++i) {
        std::string data("defdefefe");
        std::string less_data = map.compress(data, true);
    }
    std::string data("abcabcabcabcabcabc");
    std::string less_data = map.compress(data, false);
    std::string origin_data = map.uncompress(less_data);
    std::cout<<"raw:"<<data.size()<<", compress:"<<less_data.size()<<"\n";
    ASSERT_EQ(data, origin_data);
}

TEST(OnlineCompressMap, TestMapLzwBatchCache) {
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwBatchEncoder>();;
    int cache_size = 2000;
    online_compress::Map map(coder_ptr, 1000000, cache_size*2, 2);
    for (int i = 0; i < cache_size*2; ++i) {
        std::string data("abcabcabcabcabcabcabcabcabcabcabcabc");
        std::string less_data = map.compress(data, true);
    }
    for (int i = 0; i < cache_size*2; ++i) {
        std::string data("defdefefe");
        std::string less_data = map.compress(data, true);
    }
    std::string data("abcabcabcabcabcabcabcabcabcabcabcabc");
    std::string less_data = map.compress(data, false);
    std::string origin_data = map.uncompress(less_data);
    std::cout<<"raw:"<<data.size()<<", compress:"<<less_data.size()<<"\n";
    ASSERT_EQ(data, origin_data);
}

TEST(OnlineCompressMap, TestMapLzwBatchCacheReal) {
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwBatchEncoder>();;
    int cache_size = 10000;
    online_compress::Map map(coder_ptr, 32+296, cache_size*2, 2);
    for (int i = 0; i < cache_size*2; ++i) {
        std::string data("abcdefghijklmnopqrstuvwxyz");
        std::string less_data = map.compress(data, true);
        {
        std::string data("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        std::string less_data = map.compress(data, true);
        }
    }
    std::string data("abcdefghijklmnABCDEFGHIJK");
    std::string less_data = map.compress(data, false);
    std::string origin_data = map.uncompress(less_data);
    std::cout<<"raw:"<<data.size()<<", compress:"<<less_data.size()<<"\n";
    ASSERT_EQ(data, origin_data);
}

TEST(OnlineCompressMap, TestMapLzwBatchCacheFids) {
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwBatchEncoder>();;

    auto get_vec = [](int size, int range) {
        std::vector<uint16_t> vec;
        for (int i = 0; i < size; ++i) {
            vec.push_back(rand() % range);
        }
        std::sort(vec.begin(), vec.end());
        const char *p = reinterpret_cast<const char*>(vec.data());
        const size_t length = vec.size() * sizeof(uint16_t);
        return std::string(p, length);
    };
    // 这里有问题，没有解决
    int cache_size = 100000; //一百万
    online_compress::Map map(coder_ptr, cache_size, cache_size*2, 2);
    for (int i = 0; i < cache_size*2; ++i) {
        std::string p = get_vec(16, 1);
        std::string less_data = map.compress(p, true);
    }
    std::string p = get_vec(16, 1);
    std::string less_data = map.compress(p, false);
    std::string origin_data = map.uncompress(less_data);
    std::cout<<"raw:"<<p.size()<<", compress:"<<less_data.size()<<"\n";
    ASSERT_EQ(p, origin_data);
}
