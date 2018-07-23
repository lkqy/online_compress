#include <iostream> 
#include <memory> 
#include <stdlib.h> 
#include <string>
#include <vector>
#include <snappy.h>
#include <gtest/gtest.h>
#include "utils/encoder.h"


TEST(OnlineCompressEncoder, TestLzwBatchNormalUse) {
    online_compress::LzwBatchEncoder lzw(0);
    {
        std::string data = "abcabcabcabcabcabcabc";
        std::unordered_map<std::string, uint64_t> map;
        lzw.add(data, map);
        for(auto it : map) {
            std::cout<<it.first<<" "<<it.second<<"\n";
        }
    }
    {
        std::cout<<"\n\n";
        std::string data = "aaaaaa";
        std::unordered_map<std::string, uint64_t> map;
        lzw.add(data, map);
        for(auto it : map) {
            std::cout<<it.first<<" "<<it.second<<"\n";
        }
    }

    //ASSERT_EQ(dict.size(), 0);
}
