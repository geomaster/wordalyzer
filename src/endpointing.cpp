#include "endpointing.hpp"
#include <cmath>
#include <list>
#include <iostream>

using namespace wordalyzer;
using namespace std;

const int NOISE_SAMPLE_MS = 100;
const int WINDOW_SIZE_MS = 10;
const int MIN_DURATION_MS = 1000;

const int PARAM_X = 20;
const int PARAM_Y = 20;
const int PARAM_Z = 10;

namespace wordalyzer {
    struct span_t {
        size_t start, len;
    };

    float compute_noise_threshold(vector<float>::const_iterator start, vector<float>::const_iterator end);

    template<typename T>
    float compute_mean(typename vector<T>::const_iterator start, typename vector<T>::const_iterator end);

    list<span_t> to_speech_spans(const vector<bool>& is_speech);
    void lower_pits(list<span_t>& spans);
    void raise_peaks(list<span_t>& spans);
}

template<typename T>
float wordalyzer::compute_mean(typename vector<T>::const_iterator start, typename vector<T>::const_iterator end)
{
    float sample_sum = 0.0f;
    int sample_n = 0;
    for (auto it = start; it < end; it++) {
        sample_sum += abs(*it);
        sample_n++;
    }

    return sample_sum / sample_n;
}

list<span_t> wordalyzer::to_speech_spans(const vector<bool>& is_speech)
{
    if (is_speech.empty()) {
        return list<span_t>();
    }

    bool prev = is_speech[0];
    size_t start = 0;
    list<span_t> res;
    for (size_t i = 0; i < is_speech.size(); i++) {
        if (prev != is_speech[i]) {
            if (prev) {
                res.push_back({ start, i - start });
            }
            start = i;
        }
        prev = is_speech[i];
    }

    if (prev) {
        res.push_back({ start, is_speech.size() - 1 });
    }

    return res;
}

float wordalyzer::compute_noise_threshold(vector<float>::const_iterator start, vector<float>::const_iterator end)
{
    float mean = compute_mean<float>(start, end);
    float variance = 0.0f;
    int sample_n = 0;
    for (auto it = start; it < end; it++) {
        float diff = *it - mean;
        variance += diff * diff;
        sample_n++;
    }

    float std_dev = sqrt(variance / sample_n);
    return mean + 1 * std_dev;
}

void wordalyzer::raise_peaks(list<span_t>& spans)
{
    list<span_t>::iterator prev = spans.begin(),
                           second = spans.begin();
    second++;

    for (auto it = second; it != spans.end(); prev = it, it++) {
        int zeros_before = it->start - prev->start - prev->len;
        if (zeros_before < PARAM_X) {
            // See if the new sequence of 1's is going to be > Y.
            int new_ones = prev->len + zeros_before + it->len;
            if (new_ones > PARAM_Y) {
                // Join the prev and current spans
                prev->len += zeros_before + it->len;
                spans.erase(it);
                it = prev;
            }
        }
    }
}

void wordalyzer::lower_pits(list<span_t>& spans)
{
    list<span_t>::iterator prev = spans.begin();

    bool is_first = true;
    for (auto it = spans.begin(); it != spans.end();) {
        if (it->len < PARAM_Z) {
            spans.erase(it);
            if (is_first) {
                it = spans.begin();
            } else {
                it = prev;
                it++;
                is_first = false;
            }
        }  else {
            is_first = false;
            prev = it;
            it++;
        }

    }
}

vector<pair<int, int>> wordalyzer::compute_endpoints(const audio_t& audio)
{
    // If the audio is too small, just return one single piece
    if (audio.samples.size() < audio.ms_to_samples(MIN_DURATION_MS)) {
        return { { 0, audio.samples.size() - 1 } };
    }

    int noise_samples = audio.ms_to_samples(NOISE_SAMPLE_MS);
    float noise_threshold = compute_noise_threshold(audio.samples.begin(), audio.samples.begin() + noise_samples);

    int window_samples = audio.ms_to_samples(WINDOW_SIZE_MS);
    vector<bool> is_speech;
    for (int i = noise_samples; i < audio.samples.size() - window_samples; i += window_samples) {
        float mean = compute_mean<float>(audio.samples.begin() + i, audio.samples.begin() + i + window_samples);
        is_speech.push_back(mean > noise_threshold);
    }

    list<span_t> spans = to_speech_spans(is_speech);
    raise_peaks(spans);
    lower_pits(spans);

    vector<pair<int, int>> res;
    for (auto span : spans) {
        int left_w = span.start, right_w = span.start + span.len;
        res.push_back({ left_w * window_samples + noise_samples, right_w * window_samples + noise_samples - 1 });
    }

    return res;
}
