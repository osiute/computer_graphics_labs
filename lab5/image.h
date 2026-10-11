#pragma once
#include <vector>

struct Pixel {
    unsigned char r, g, b;
};

struct Image {
    int width = 0;
    int height = 0;
    std::vector<Pixel> pixels;
};

struct HSV {
    double h; // Оттенок [0, 360]
    double s; // Насыщенность [0, 1]
    double v; // Яркость [0, 1]
};

// Функции конвертации цветовых моделей
HSV rgb_to_hsv(Pixel p);
Pixel hsv_to_rgb(HSV hsv);