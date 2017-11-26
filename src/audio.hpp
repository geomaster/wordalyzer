#pragma once
#include <string>
#include <vector>

#include "common.hpp"

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

    std::vector<byte> serialize_word(const word_t& word);
    word_t deserialize_word(const std::vector<byte>& bytes);
}
