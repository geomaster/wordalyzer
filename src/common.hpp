#pragma once
#include <vector>

namespace wordalyzer {
    typedef unsigned char byte;

    void serialize_size(size_t sz, std::vector<byte>& dest);
    void serialize_double(double d, std::vector<byte>& dest);

    size_t deserialize_size(std::vector<byte>::const_iterator& it);
    double deserialize_double(std::vector<byte>::const_iterator& it);
}
