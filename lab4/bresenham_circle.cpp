#include "lab4.h"

// Рисует окружность алгоритмом Брезенхема
void draw_circle_bresenham(Image& img, int cx, int cy, int radius, Pixel color) {
    if (radius <= 0) {
        return;
    }

    int x = 0;
    int y = radius;
    int delta = 2 - 2 * radius;

    while (x <= y) {
        plot_octants(img, cx, cy, x, y, color);
        int d1 = 2 * (delta + y) - 1;
        int d2 = 2 * (delta - x) - 1;
        if (delta < 0 && d1 <= 0) {
            ++x;
            delta = delta + 2 * x + 1;
        } else if (delta > 0 && d2 > 0) {
            --y;
            delta = delta - 2 * y + 1;
        } else {
            ++x;
            --y;
            delta = delta + 2 * (x - y);
        }
    }
}
