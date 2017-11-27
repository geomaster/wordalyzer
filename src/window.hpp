#pragma once
#include <vector>

namespace wordalyzer {
    void apply_hamming_window(std::vector<float>& samples);
    void apply_hann_window(std::vector<float>& samples);

    float get_hamming_window_gain();
    float get_hann_window_gain();
}
