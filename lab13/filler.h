#pragma once

#include <vector>
#include "image.h"
#include "geometry.h"

void rasterize_polygon_fill(Image& img, const std::vector<Point2D>& polygon, const Pixel& color);