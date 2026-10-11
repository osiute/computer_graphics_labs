#include "image.h"
#include <algorithm>
#include <cmath>

HSV rgb_to_hsv(Pixel p) {
    double r = p.r / 255.0;
    double g = p.g / 255.0;
    double b = p.b / 255.0;

    double max_val = std::max({r, g, b});
    double min_val = std::min({r, g, b});
    double delta = max_val - min_val;

    double h = 0.0;
    double s = 0.0;
    double v = max_val;

    if (delta > 1e-5) {
        if (max_val == r) {
            h = 60.0 * fmod((g - b) / delta, 6.0);
        } else if (max_val == g) {
            h = 60.0 * ((b - r) / delta + 2.0);
        } else {
            h = 60.0 * ((r - g) / delta + 4.0);
        }
        if (h < 0.0) {
            h += 360.0;
        }
    }

    if (max_val > 1e-5) {
        s = delta / max_val;
    }

    return {h, s, v};
}

Pixel hsv_to_rgb(HSV hsv) {
    double h = hsv.h;
    double s = hsv.s;
    double v = hsv.v;

    double c = v * s;
    double x = c * (1.0 - std::abs(fmod(h / 60.0, 2.0) - 1.0));
    double m = v - c;

    double r = 0.0, g = 0.0, b = 0.0;

    if (0.0 <= h && h < 60.0) {
        r = c; g = x; b = 0.0;
    } else if (60.0 <= h && h < 120.0) {
        r = x; g = c; b = 0.0;
    } else if (120.0 <= h && h < 180.0) {
        r = 0.0; g = c; b = x;
    } else if (180.0 <= h && h < 240.0) {
        r = 0.0; g = x; b = c;
    } else if (240.0 <= h && h < 300.0) {
        r = x; g = 0.0; b = c;
    } else {
        r = c; g = 0.0; b = x;
    }

    unsigned char red = static_cast<unsigned char>(std::round((r + m) * 255.0));
    unsigned char green = static_cast<unsigned char>(std::round((g + m) * 255.0));
    unsigned char blue = static_cast<unsigned char>(std::round((b + m) * 255.0));

    return {red, green, blue};
}