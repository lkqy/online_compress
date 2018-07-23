#pragma once

#include <atomic>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include "utils/util.h"

namespace online_compress {

// 流式加入，流式更新
// 线程安全
class Encoder {
 public:
    Encoder(size_t _cappacity) : cappacity(_cappacity){};
    virtual ~Encoder(){};
    virtual bool add(const std::string& line,
                     std::unordered_map<std::string, uint64_t>& result) = 0;

 protected:
    std::atomic<size_t> cappacity;
    tbb::concurrent_hash_map<std::string, MyAtomic<size_t>> map;
};

typedef std::shared_ptr<Encoder> EncoderPtr;

class LzwBatchEncoder : public Encoder {
 public:
    LzwBatchEncoder(size_t _cappacity) : Encoder(_cappacity){};
    bool add(const std::string& line, std::unordered_map<std::string, uint64_t>& result);
};

}  // namespace online_compress
