#pragma once
#include <vector>

#include "audio.hpp"

namespace wordalyzer {
    std::vector<std::pair<int, int>> compute_endpoints(const audio_t& audio);
}
