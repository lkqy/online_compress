#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace online_compress {

/*
   约定0~255是保护字段，不可以用于编码
*/

class Encoder {
 public:
    virtual ~Encoder(){};
    virtual std::unordered_map<std::string, uint64_t> build_dict(
        const std::vector<std::string>& datas) = 0;
};

typedef std::shared_ptr<Encoder> EncoderPtr;

class LzwEncoder : public Encoder {
 public:
    virtual std::unordered_map<std::string, uint64_t> build_dict(
        const std::vector<std::string>& datas);
};

class LzwBatchEncoder : public Encoder {
 public:
    virtual std::unordered_map<std::string, uint64_t> build_dict(
        const std::vector<std::string>& datas);
};

}  // namespace online_compress
