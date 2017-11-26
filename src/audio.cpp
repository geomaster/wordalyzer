#include "audio.hpp"
#include <cassert>

using namespace wordalyzer;
using namespace std;

vector<byte> wordalyzer::serialize_word(const word_t& word)
{
    vector<byte> res;
    serialize_size(word.coeff_vectors.size(), res);
    if (!word.coeff_vectors.empty()) {
        serialize_size(word.coeff_vectors[0].size(), res);
        for (const auto& v : word.coeff_vectors) {
            assert(v.size() == word.coeff_vectors[0].size() && "Mismatching sizes for coefficient vectors");
            for (double d : v) {
                serialize_double(d, res);
            }
        }
    }

    return res;
}

word_t wordalyzer::deserialize_word(const vector<byte>& bytes)
{
    word_t res;
    auto it = bytes.begin();
    size_t remaining_bytes = bytes.size();

    assert(remaining_bytes >= sizeof(size_t) && "Not enough bytes");
    size_t count = deserialize_size(it);
    remaining_bytes -= sizeof(size_t);

    if (count > 0) {
        assert(remaining_bytes >= sizeof(size_t) && "Not enough bytes");
        size_t v_size = deserialize_size(it);
        remaining_bytes -= sizeof(size_t);

        if (v_size > 0) {
            assert(remaining_bytes >= count * v_size * sizeof(double) && "Not enough bytes");

            res.coeff_vectors.resize(count);
            for (size_t i = 0; i < count; i++) {
                res.coeff_vectors[i].resize(v_size);
                for (size_t j = 0; j < v_size; j++) {
                    res.coeff_vectors[i][j] = deserialize_double(it);
                }
            }
        } else {
            res.coeff_vectors.resize(count);
        }
    }

    return res;
}
