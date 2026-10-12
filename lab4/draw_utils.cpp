#include "lab4.h"
#include <algorithm>
#include <cmath>

// Ставит пиксель в точку (x, y)
void set_pixel(Image& img, int x, int y, Pixel color) {
    if (x < 0 || y < 0 || x >= img.width || y >= img.height) {
        return;
    }
    img.pixels[y * img.width + x] = color;
}

// Рисует отрезок из (x0, y0) в (x1, y1)
void draw_line(Image& img, int x0, int y0, int x1, int y1, Pixel color) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    bool steep = dy > dx;
    if (steep) {
        std::swap(dx, dy);
    }

    int err = 2 * dy - dx;
    int x = x0;
    int y = y0;
    for (int i = 0; i <= dx; ++i) {
        set_pixel(img, x, y, color);
        if (err >= 0) {
            if (steep) {
                x += sx;
            } else {
                y += sy;
            }
            err -= 2 * dx;
        }
        if (steep) {
            y += sy;
        } else {
            x += sx;
        }
        err += 2 * dy;
    }
}

// Ставит 4 симметричные точки окружности
void plot_quadrants(Image& img, int cx, int cy, int x, int y, Pixel color) {
    set_pixel(img, cx + x, cy + y, color);
    set_pixel(img, cx + x, cy - y, color);
    set_pixel(img, cx - x, cy + y, color);
    set_pixel(img, cx - x, cy - y, color);
}

// Ставит 8 симметричных точек окружности
void plot_octants(Image& img, int cx, int cy, int x, int y, Pixel color) {
    plot_quadrants(img, cx, cy, x, y, color);
    plot_quadrants(img, cx, cy, y, x, color);
}

// Рисует крест, делящий кадр на 4 части
void draw_split(Image& img, Pixel color) {
    int cx = img.width / 2;
    int cy = img.height / 2;
    draw_line(img, cx, 0, cx, img.height - 1, color);
    draw_line(img, 0, cy, img.width - 1, cy, color);
}
