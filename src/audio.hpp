#pragma once
#include <string>
#include <vector>

#include "common.hpp"
#include <exception>

namespace wordalyzer {
    struct clip_t;

    struct word_t {
        std::vector<std::vector<double>> coeff_vectors;
    };

    struct clip_t {
        std::string name;
        std::vector<word_t> words;

        int vector_size;
        int window_size;
        int window_stride;
    };

    struct audio_t {
        std::vector<float> samples;
        int sample_rate;

        int ms_to_samples(int ms) const
        {
            return ms / 1000.0f * sample_rate;
        }

        int s_to_samples(float s) const
        {
            return s * sample_rate;
        }

        int samples_to_ms(int sample) const
        {
            return 1000.0f * sample / sample_rate;
        }

        float samples_to_s(int sample) const
        {
            return static_cast<float>(sample) / sample_rate;
        }
    };

    std::vector<byte> serialize_word(const word_t& word);
    word_t deserialize_word(const std::vector<byte>& bytes);
}
