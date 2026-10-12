#include "lab4.h"
#include <cmath>

// Считает радиус описанной окружности по стороне многоугольника
double circumradius_from_side(double side, int n) {
    const double pi = 3.141592653589793;
    return side / (2.0 * std::sin(pi / n));
}

// Считает радиус вписанной окружности по стороне многоугольника
double inradius_from_side(double side, int n) {
    const double pi = 3.141592653589793;
    return side / (2.0 * std::tan(pi / n));
}

// Рисует правильный пятиугольник
void draw_regular_pentagon(Image& img, int cx, int cy, int circum_r, Pixel color) {
    const double pi = 3.141592653589793;
    int xs[5];
    int ys[5];
    for (int i = 0; i < 5; ++i) {
        double ang = -pi / 2.0 + i * 2.0 * pi / 5.0;
        xs[i] = static_cast<int>(std::round(cx + circum_r * std::cos(ang)));
        ys[i] = static_cast<int>(std::round(cy + circum_r * std::sin(ang)));
    }
    for (int i = 0; i < 5; ++i) {
        draw_line(img, xs[i], ys[i], xs[(i + 1) % 5], ys[(i + 1) % 5], color);
    }
}
