#include "diff_diagram.hpp"
#include <cmath>

using namespace wordalyzer;
using namespace wordalyzer::gui;
using namespace std;

diff_diagram::diff_diagram(vector<vector<double>>::const_iterator begin1,
                           vector<vector<double>>::const_iterator end1,
                           vector<vector<double>>::const_iterator begin2,
                           vector<vector<double>>::const_iterator end2)
{
    max_diff = -1.0;
    for (auto it1 = begin1, it2 = begin2; it1 < end1 && it2 < end2; it1++, it2++) {
        double dist_sq = 0.0;
        const auto& v1 = *it1, &v2 = *it2;
        for (int i = 0; i < v1.size(); i++) {
            dist_sq += v1[i] * v1[i] + v2[i] * v2[i];
        }

        double dist = sqrt(dist_sq);
        if (dist > max_diff) {
            max_diff = dist;
        }
        diffs.push_back(sqrt(dist_sq));
    }
}

map<float, string> diff_diagram::get_y_labels()
{
}
