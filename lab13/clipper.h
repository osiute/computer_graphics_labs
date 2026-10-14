#pragma once

#include <vector>
#include "geometry.h"

std::vector<Point2D> sutherland_hodgman_clip(const std::vector<Point2D>& subject_polygon, 
                                             const std::vector<Point2D>& clip_window);