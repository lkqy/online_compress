#include <iostream> 
#include <memory> 
#include <stdlib.h> 
#include <string>
#include <vector>
#include <snappy.h>
#include <gtest/gtest.h>
#include "utils/encoder.h"


TEST(OnlineCompressEncoder, TestLzwNormalUse) {
    online_compress::LzwEncoder lzw;
    {
        std::vector<std::string> data = {"abcabcabcabcabcabcabc"};
        auto map = lzw.build_dict(data);
        for(auto it : map) {
            std::cout<<it.first<<" "<<it.second<<"\n";
        }
    }
    {
        std::cout<<"\n\n";
        std::vector<std::string> data = {"aaaaaa"};
        auto map = lzw.build_dict(data);
        for(auto it : map) {
            std::cout<<it.first<<" "<<it.second<<"\n";
        }
    }

    //ASSERT_EQ(dict.size(), 0);
}
