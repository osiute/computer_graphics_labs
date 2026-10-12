#pragma once
#include <vector>
#include "image.h"

struct Point2D {
    double x, y;
};

// 1. Квадратичная кривая Безье (по 3 точкам)
std::vector<Point2D> generate_quadratic_bezier(const std::vector<Point2D>& control_points, int steps);

// 2. Кубическая кривая Безье (по 4 точкам)
std::vector<Point2D> generate_cubic_bezier(const std::vector<Point2D>& control_points, int steps);

// 3. Кривая Чайкина (метод углового отсечения по произвольному полигону)
std::vector<Point2D> generate_chaikin(const std::vector<Point2D>& control_points, int iterations);