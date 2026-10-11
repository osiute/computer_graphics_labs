#pragma once

#include <vector>

struct Point {
    // Координаты в пикселях
    int x;
    int y;
};

struct Segment {
    Point p1;
    Point p2;
};

std::vector<Segment> get_pentagram_segments(int cx, int cy, int radius, double initial_angle_deg);