#pragma once

#include <string>
#include <vector>
#include "geometry.h"

void save_segments_to_svg(const std::string& filename, const std::vector<Segment>& segments, int width, int height);

std::vector<Segment> read_segments_from_svg(const std::string& filename);