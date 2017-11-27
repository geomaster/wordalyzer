#include <armadillo>
#include "lpc.hpp"

using namespace wordalyzer;
using namespace std;

namespace wordalyzer {
    vector<double> analyze_window(const vector<float>& samples, int p);
    double autocorrelate(const vector<float>& samples, int k);
}

double wordalyzer::autocorrelate(const vector<float>& samples, int k)
{
    double res = 0.0;
    for (size_t i = k; i < samples.size(); i++) {
        res += static_cast<double>(samples[i]) * static_cast<double>(samples[i - k]);
    }

    return res;
}

vector<double> wordalyzer::analyze_window(const vector<float>& samples, int p)
{
    vector<double> R(p + 1);
    for (int i = 0; i <= p; i++) {
        R[i] = autocorrelate(samples, i);
    }

    arma::mat M(p, p);

    // Fill the matrix
    for (int i = 0; i < p; i++) {
        // Diagonal elements are R(0)
        M(i, i) = R[0];

        // Left from the diagonal
        for (int j = i + 1; j < p; j++) {
            M(i, j) = R[j - i];
        }

        // Right from the diagonal
        for (int j = 0; j < i; j++) {
            M(i, j) = R[i - j];
        }
    }

    arma::mat v(p, 1);
    for (int i = 0; i < p; i++) {
        v(i, 0) = R[i + 1];
    }

    arma::mat coeffs = arma::inv(M) * v;

    vector<double> res(p);
    for (int i = 0; i < p; i++) {
        res[i] = coeffs(i, 0);
    }

    return res;
}

word_t wordalyzer::analyze_word(std::vector<float>::const_iterator begin,
                                std::vector<float>::const_iterator end,
                                int window_size,
                                int window_stride,
                                int vector_size,
                                WindowFunction window_fn)
{
    vector<float> window(window_size);
    word_t res;
    for (auto it = begin + window_size / 2; it < end; it += window_stride) {
        for (int i = 0; i < window_size; i++) {
            window[i] = *(it + i - window_size / 2);
        }
        apply_window(window_fn, window);
        for (int i = 0; i < window_size; i++) {
            window[i] *= 1.0f / get_window_gain(window_fn);
        }

        res.coeff_vectors.push_back(analyze_window(window, vector_size));
    }

    return res;
}

