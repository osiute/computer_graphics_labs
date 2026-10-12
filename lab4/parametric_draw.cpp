#include "lab4.h"
#include <algorithm>
#include <cmath>

// Рисует окружность по параметрическим уравнениям
void draw_circle_parametric(Image& img, int cx, int cy, int radius, Pixel color) {
    if (radius <= 0) {
        return;
    }

    const double pi = 3.141592653589793;
    int steps = std::max(8, static_cast<int>(std::ceil(2.0 * pi * radius)));
    int prev_x = cx + radius;
    int prev_y = cy;

    for (int i = 1; i <= steps; ++i) {
        double t = 2.0 * pi * i / steps;
        int x = static_cast<int>(std::round(cx + radius * std::cos(t)));
        int y = static_cast<int>(std::round(cy + radius * std::sin(t)));
        draw_line(img, prev_x, prev_y, x, y, color);
        prev_x = x;
        prev_y = y;
    }
}
