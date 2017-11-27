#pragma once
#include <vector>

namespace wordalyzer {
    enum WindowFunction {
        WINDOW_NONE,
        WINDOW_HAMMING,
        WINDOW_HANN
    };

    void apply_hamming_window(std::vector<float>& samples);
    void apply_hann_window(std::vector<float>& samples);
    void apply_window(WindowFunction fn, std::vector<float>& samples);

    float get_hamming_window_gain();
    float get_hann_window_gain();
    float get_window_gain(WindowFunction fn);
}
