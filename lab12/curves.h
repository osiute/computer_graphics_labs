#pragma once
#include <vector>
#include "geometry.h"

// Кубическая Безье по 4 точкам, t ∈ [0,1]
Point2D cubic_bezier_point(const Point2D& p0, const Point2D& p1,
                           const Point2D& p2, const Point2D& p3, double t);

// Кусочная кубическая Безье: сегменты (P0..P3), (P3..P6), ...
std::vector<Point2D> generate_cubic_bezier(const std::vector<Point2D>& control_points, int steps);

// Чайкин: Q=3/4 Pi + 1/4 Pi+1, R=1/4 Pi + 3/4 Pi+1
std::vector<Point2D> generate_chaikin(const std::vector<Point2D>& control_points, int iterations);

// Слои де Кастельжо в момент t (промежуточные вершины)
std::vector<std::vector<Point2D>> decasteljau_layers(const std::vector<Point2D>& points, double t);
