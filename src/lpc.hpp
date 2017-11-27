#pragma once
#include <vector>

#include "audio.hpp"
#include "window.hpp"

namespace wordalyzer {
    word_t analyze_word(std::vector<float>::const_iterator begin,
                        std::vector<float>::const_iterator end,
                        int window_size,
                        int window_stride,
                        int vector_size,
                        WindowFunction window_fn);
}
