#include "diff_diagram.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace wordalyzer;
using namespace wordalyzer::gui;
using namespace std;

const int Y_LABEL_COUNT = 10;
const int MIN_X_WIDTH = 2;

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
            dist_sq += (v1[i] - v2[i]) * (v1[i] - v2[i]);
        }

        double dist = sqrt(dist_sq);
        if (dist > max_diff) {
            max_diff = dist;
        }
        diffs.push_back(dist);
    }

    left_i = 0;
    right_i = diffs.size() - 1;
}

map<float, string> diff_diagram::get_y_labels()
{
    map<float, string> result;
    for (int i = 0; i < Y_LABEL_COUNT; i++) {
        float alpha = static_cast<float>(i) / (Y_LABEL_COUNT - 1);
        double value = alpha * max_diff;
        ostringstream s;
        s << fixed << setprecision(2) << value;
        result[alpha] = s.str();
    }

    return result;
}

pair<float, float> diff_diagram::get_full_x_range()
{
    return { 0, diffs.size() - 1 };
}

float diff_diagram::get_min_x_width()
{
    return MIN_X_WIDTH;
}

float diff_diagram::get_x_granularity(float)
{
    return 1;
}

map<float, string> diff_diagram::get_x_labels()
{
    map<float, string> result;
    for (int i = left_i; i <= right_i; i++) {
        float alpha = (i - left_i) / static_cast<float>(right_i - left_i);
        result[alpha] = to_string(i);
    }

    return result;
}

float diff_diagram::get_drag_step_normalized()
{
    return 1.0f / (right_i - left_i);
}

void diff_diagram::set_x_range(pair<float, float> new_range)
{
    left_i = new_range.first;
    right_i = new_range.second;
}

void diff_diagram::draw(sf::RenderTarget* target, pair<int, int> bottom_left, pair<int, int> size)
{
    for (int i = left_i + 1; i <= right_i; i++) {
        float x1 = bottom_left.first + size.first * static_cast<float>(i - 1 - left_i) / (right_i - left_i);
        float x2 = bottom_left.first + size.first * static_cast<float>(i - left_i) / (right_i - left_i);
        float y1 = bottom_left.second - diffs[i - 1] / max_diff * size.second;
        float y2 = bottom_left.second - diffs[i] / max_diff * size.second;

        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(x1, y1)),
            sf::Vertex(sf::Vector2f(x2, y2))
        };

        target->draw(line, 2, sf::Lines);
    }
}
