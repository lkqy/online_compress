#include "utils/encoder.h"
#include <iostream>

namespace online_compress {

std::unordered_map<std::string, uint64_t> LzwEncoder::build_dict(
    const std::vector<std::string>& datas) {
    std::ostringstream oss;
    for (auto& s : datas) {
        oss << s;
    }
    std::string data = oss.str();
    std::unordered_map<std::string, uint64_t> map;
    std::string p;
    for (auto c : data) {
        std::string n = p + c;
        if (map.find(n) != map.end()) {
            map[n] += 1;
            p = n;
        } else {
            map[n] = 1;
            p = c;
        }
    }
    return map;
}

std::unordered_map<std::string, uint64_t> LzwBatchEncoder::build_dict(
    const std::vector<std::string>& datas) {
    std::unordered_map<std::string, uint64_t> map;
    for (auto& data : datas) {
        std::string p;
        for (auto c : data) {
            std::string n = p + c;
            if (map.find(n) != map.end()) {
                map[n] += 1;
                p = n;
            } else {
                map[n] = 1;
                p = c;
            }
        }
    }
    return map;
}

}  // namespace online_compress
