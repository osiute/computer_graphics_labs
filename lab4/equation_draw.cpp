#include "lab4.h"
#include <cmath>

// Рисует окружность по уравнению x^2 + y^2 = R^2
void draw_circle_equation(Image& img, int cx, int cy, int radius, Pixel color) {
    if (radius <= 0) {
        return;
    }

    int x = 0;
    int y = radius;
    while (x < y) {
        y = static_cast<int>(std::round(std::sqrt(static_cast<double>(radius) * radius - static_cast<double>(x) * x)));
        plot_quadrants(img, cx, cy, x, y, color);
        ++x;
    }

    y = radius;
    while (y >= 0) {
        x = static_cast<int>(std::round(std::sqrt(static_cast<double>(radius) * radius - static_cast<double>(y) * y)));
        plot_quadrants(img, cx, cy, x, y, color);
        --y;
    }
}
