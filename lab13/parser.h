#pragma once

#include <vector>
#include <string>
#include "geometry.h"

struct scene_data {
    int canvas_width = 500;
    int canvas_height = 500;
    std::vector<Point2D> subject_polygon;
    std::vector<Point2D> clip_window;
};

bool load_scene_from_file(const std::string& filename, scene_data& data);