#include "window.hpp"
#include <cmath>

using namespace wordalyzer;
using namespace std;

namespace wordalyzer {
    struct hamming_window {
        float operator()(float alpha) const;
    };

    struct hann_window {
        float operator()(float alpha) const;
    };

    template<typename Window>
    void apply_window(vector<float>& samples);
}

template<typename Window>
void wordalyzer::apply_window(vector<float>& samples)
{
    Window w;
    for (size_t i = 0; i < samples.size(); i++) {
        float alpha = static_cast<float>(i) / (samples.size() - 1);
        samples[i] *= w(alpha);
    }
}

const float HAMMING_ALPHA = 0.54f;
const float HAMMING_BETA = 1.0f - HAMMING_ALPHA;

float wordalyzer::hamming_window::operator()(float alpha) const
{
    return HAMMING_ALPHA - HAMMING_BETA * cos(2 * M_PI * alpha);
}

float wordalyzer::hann_window::operator()(float alpha) const
{
    return pow(sin(M_PI * alpha), 2.0f);
}

void wordalyzer::apply_hamming_window(vector<float>& samples)
{
    apply_window<hamming_window>(samples);
}

void wordalyzer::apply_hann_window(vector<float>& samples)
{
    apply_window<hann_window>(samples);
}

float wordalyzer::get_hamming_window_gain()
{
    return (HAMMING_ALPHA + HAMMING_BETA) / 2.0f;
}

float wordalyzer::get_hann_window_gain()
{
    return 0.5f;
}
