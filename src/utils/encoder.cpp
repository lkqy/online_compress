#include "utils/encoder.h"
#include <iostream>

namespace online_compress {

bool LzwBatchEncoder::add(const std::string& line,
                          std::unordered_map<std::string, uint64_t>& result) {
    std::string p;
    for (auto c : line) {
        std::string n = p + c;
        typename tbb::concurrent_hash_map<std::string, MyAtomic<size_t>>::accessor a;
        if (map.find(a, n)) {
            a->second.inr(1);
        } else if (map.insert(a, n)) {
            //    a->second = 1;
        }
    }
    if (map.size() > cappacity) {
        for (auto it = map.begin(); it != map.end(); ++it) {
            result[it->first] = it->second.load();
        }
        map.clear();
        return true;
    }
    return false;
}

}  // namespace online_compress
