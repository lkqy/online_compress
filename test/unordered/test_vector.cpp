#include <iostream> 
#include <memory> 
#include <stdlib.h> 
#include <string>
#include <vector>
#include <snappy.h>
#include <gtest/gtest.h>
#include "unordered/snappy.h"

template<typename T>
std::string test_snappy(std::vector<T>& vec) {
    std::sort(vec.begin(), vec.end());
    char *input = new char[vec.size() * sizeof(T)];
    T *f = reinterpret_cast<T*>(input);
    for (auto v : vec) {
        *f ++ = v;
    }
    T length = reinterpret_cast<char*>(f) - input;
    std::string output;
    snappy::Compress(input, length, &output);
    return output;
}

template<typename T>
void test() {
    online_compress::unordered::VectorCompress<T> vc;
    for (int i = 256; i < 1024 * 256; i *= 2) {
        std::vector<T> vec;
        for (int j = 0; j < i; ++j) {
            vec.push_back(rand()%1024);
        }
        std::string output = vc.compress(vec);
    }
    for (int i = 64; i < 1024 ; i *= 2) {
        std::vector<T> vec;
        for (int j = 0; j < i; ++j) {
            vec.push_back(rand()%1024);
        }
        std::string output = vc.compress(vec);
        std::cout<<typeid(T).name()<<", "
            <<"vec_"<<i<<", real_size:"<<vec.size() * sizeof(T)<<", compress_size:"<<output.length()<<"\n";
        std::sort(vec.begin(), vec.end());
        std::vector<T> result = vc.uncompress(output);
        for (int j = 0; i < i; ++j) {
            ASSERT_EQ(result[j], vec[j]);
        }

    }
}

template<typename T>
void test_lookup() {
    online_compress::unordered::VectorCompress<T> vc;
    for (int i = 256; i < 1024 * 256 * 4; i *= 2) {
        std::vector<T> vec;
        for (int j = 0; j < i; ++j) {
            vec.push_back(rand()%1024);
        }
        std::string output = vc.compress(vec);
    }
    for (int i = 64; i < 10240 ; i *= 2) {
        std::vector<T> vec;
        for (int j = 0; j < i; ++j) {
            vec.push_back(rand()%1024);
        }
        std::string output = vc.compress(vec, true);
        std::cout<<typeid(T).name()<<"\t"
            <<i<<"\t\t"<<vec.size() * sizeof(T)<<"\t\t"<<output.length()
            <<"\t\t"<<test_snappy<T>(vec).length()<<"\n";
        std::sort(vec.begin(), vec.end());
        std::vector<T> result = vc.uncompress(output);
        for (int j = 0; i < i; ++j) {
            ASSERT_EQ(result[j], vec[j]);
        }
    }
}

TEST(SnappyTest, TestSortAsc) {
    std::vector<int> v = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    std::random_shuffle(v.begin(), v.end());
    std::sort(v.begin(), v.end(), [](int i, int j) { return i < j;});
    std::cout<<"test desc sort:\n";
    for (auto i : v) {
        std::cout<<i<<" ";
    }
    std::cout<<"\n";
    ASSERT_EQ(3, v[3]);
}
TEST(SnappyTest, TestPartialSortDesc) {
    std::vector<int> v = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    std::random_shuffle(v.begin(), v.end());
    std::partial_sort(v.begin(), v.begin() + 3, v.end(), [](int i, int j) { return i >= j;});
    std::cout<<"test desc sort:\n";
    for (auto i : v) {
        std::cout<<i<<" ";
    }
    std::cout<<"\n";
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 3; j < v.size(); ++j) {
            ASSERT_GT(v[i], v[j]);
        }
    }
}

TEST(SnappyTest, TestSimple) {
    test<int>();
    test<long>();
    test<double>();
}


TEST(SnappyTest, TestLookUp) {
    test_lookup<int>();
    test_lookup<long>();
    test_lookup<double>();
}

TEST(SnappyTest, TestRawSnappy) {
    std::vector<int> all;
    for (int i = 0; i < 16; i ++) {
        std::vector<int> vec;
        for (int j = 0; j < 64; ++j) {
            vec.push_back(rand()%1024);
        }
        for (int j = 0; j < 10; ++j) {
            vec.push_back(rand());
        }
        //std::sort(vec.begin(), vec.end());

        for (auto v : vec) {
            all.push_back(v);
        }
    }
    std::string result = test_snappy<int>(all);
    std::cout<<"multi-item compress:"<<all.size() * sizeof(int)<<"\t\t"<<result.length()<<"\n";
}
