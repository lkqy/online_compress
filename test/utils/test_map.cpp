#include <iostream> 
#include <memory> 
#include <algorithm> 
#include <fstream> 
#include <stdlib.h> 
#include <string>
#include <vector>
#include <snappy.h>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include "utils/encoder.h"
#include "utils/map.h"


TEST(OnlineCompressMap, TestMapNormalUse) {
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwBatchEncoder>(100);;
    online_compress::Map map(coder_ptr, 1000000, 2);
    std::string data("abcabcabcabcabcabc");
    std::string less_data = map.compress(data, false);
    std::string origin_data = map.uncompress(less_data);
    ASSERT_EQ(data, origin_data);
    std::cout<<"raw:"<<data.size()<<", compress:"<<less_data.size()<<"\n";
}

TEST(OnlineCompressMap, TestMapCache) {
    int cache_size = 2000;
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwBatchEncoder>(cache_size*2);;
    online_compress::Map map(coder_ptr, 1000000, 2);
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
    int cache_size = 2000;
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwBatchEncoder>(cache_size*2);;
    online_compress::Map map(coder_ptr, 1000000, 2);
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
    int cache_size = 10000;
    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwBatchEncoder>(cache_size*2);;
    online_compress::Map map(coder_ptr, 32+296, 2);
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

TEST(OnlineCompressMap, TestMapLzwBatchCacheOnRealPushFids) {
    std::ifstream fin("data/fids.txt");
    std::string line;
    std::vector<std::string> array;
    while (!fin.eof()) {
        std::getline(fin, line);
        std::vector<std::string> vec;
        boost::split(vec, line, boost::is_any_of(","));
        if (vec.size() == 0) continue;
        std::vector<uint64_t> fids;
        for (auto& s : vec) {
            if (s.empty()) continue;
            std::string::size_type sz = 0; 
            //std::cout<<s<<"\n";
            uint64_t v = std::stoull(s, &sz, 10);
            fids.push_back(v);
        }
        //const uint64_t *d = fids.data();
        const char* p = reinterpret_cast<const char *>(fids.data());;
        const size_t size = sizeof(uint64_t) * fids.size();
        std::string s(p, size);
        //array.push_back(line);
        array.push_back(s);
    }

    online_compress::EncoderPtr coder_ptr = std::make_shared<online_compress::LzwBatchEncoder>(array.size());;
    online_compress::Map map(coder_ptr, array.size(), 2);
    for (auto& s : array) {
        map.compress(s, true);
        map.compress(s, true);
    }
    auto & raw_data = array[0];
    std::string less_data = map.compress(raw_data, false);
    std::string origin_data = map.uncompress(less_data);
    std::cout<<"raw:"<<raw_data.size()<<", compress:"<<less_data.size()<<"\n";
    ASSERT_EQ(raw_data, origin_data);
}
