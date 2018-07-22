#include <iostream> 
#include <memory> 
#include <stdlib.h> 
#include <string>
#include <vector>
#include <snappy.h>
#include <gtest/gtest.h>
#include "utils/dictionary.h"


TEST(OnlineCompressDictionary, TestNormalUse) {
    online_compress::Dictionary<int, int> dict;
    int size = 100;
    for (int i = 0; i < size; ++i) {
        dict.add(i, i);
        ASSERT_TRUE(dict.exists(i, i));
        ASSERT_TRUE(dict.exists_first(i));
        ASSERT_TRUE(dict.exists_second(i));
    }
    ASSERT_EQ(dict.size(), size);
    for (int i = 0; i < size; ++i) {
        dict.erase(i, i);
        ASSERT_FALSE(dict.exists(i, i));
        ASSERT_FALSE(dict.exists_first(i));
        ASSERT_FALSE(dict.exists_second(i));
    }
    ASSERT_EQ(dict.size(), 0);
}

TEST(OnlineCompressDictionary, TestDifferentType) {
    online_compress::Dictionary<int, long> dict;
    int size = 100;
    for (int i = 0; i < size; ++i) {
        long j = i * i;
        dict.add(i, j);
        ASSERT_TRUE(dict.exists(i, j));
        ASSERT_TRUE(dict.exists_first(i));
        ASSERT_TRUE(dict.exists_second(j));
    }
    ASSERT_EQ(dict.size(), size);
    for (int i = 0; i < size; ++i) {
        long j = i * i;
        dict.erase(i, j);
        ASSERT_FALSE(dict.exists(i, j));
        ASSERT_FALSE(dict.exists_first(i));
        ASSERT_FALSE(dict.exists_second(j));
    }
    ASSERT_EQ(dict.size(), 0);
}
