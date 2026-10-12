#include "lab4.h"

#define cimg_display 0
#include <io.h>
#include <fcntl.h>
#include <cstdio>
#include <ctime>
#include <cwchar>

#ifndef _fileno
#define _fileno fileno
#endif

#ifndef localtime_s
inline int localtime_s(std::tm* result, const std::time_t* timep) {
    std::tm* t = std::localtime(timep);
    if (!t) {
        return 1;
    }
    *result = *t;
    return 0;
}
#endif

#include "../libraries/CImg.h"

using namespace cimg_library;

// Рисует окружность функцией CImg::draw_circle
void draw_circle_cimg(Image& img, int cx, int cy, int radius, Pixel color) {
    if (radius <= 0 || img.width <= 0 || img.height <= 0) {
        return;
    }

    CImg<unsigned char> buf(img.width, img.height, 1, 3);
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            const Pixel& p = img.pixels[y * img.width + x];
            buf(x, y, 0, 0) = p.r;
            buf(x, y, 0, 1) = p.g;
            buf(x, y, 0, 2) = p.b;
        }
    }

    const unsigned char col[] = {color.r, color.g, color.b};
    buf.draw_circle(cx, cy, radius, col, 1.0f, ~0U);

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            img.pixels[y * img.width + x] = {
                buf(x, y, 0, 0),
                buf(x, y, 0, 1),
                buf(x, y, 0, 2)
            };
        }
    }
}
