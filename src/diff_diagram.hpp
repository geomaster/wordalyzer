#pragma once
#include "gui.hpp"

namespace wordalyzer::gui {
    class diff_diagram : public diagram {
    private:
        std::vector<double> diffs;
        double max_diff;

    public:
        diff_diagram(std::vector<std::vector<double>>::const_iterator begin1,
                     std::vector<std::vector<double>>::const_iterator end1,
                     std::vector<std::vector<double>>::const_iterator begin2,
                     std::vector<std::vector<double>>::const_iterator end2);

        std::map<float, std::string> get_y_labels();
        std::string get_title() { return "Coefficient vector diff"; }
        std::string get_message() { return ""; }

        std::pair<float, float> get_full_x_range();
        float get_min_x_width();
        float get_x_granularity(float min_drag_step);
        std::map<float, std::string> get_x_labels();

        float get_drag_step_normalized();

        float get_zoom_granularity();

        void set_x_range(std::pair<float, float> new_range);

        void draw(sf::RenderTarget* target, std::pair<int, int> bottom_left, std::pair<int, int> size);

        virtual ~diff_diagram() {}
    };
}
