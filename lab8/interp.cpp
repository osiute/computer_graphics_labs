#include "lab8.h"
#include <cmath>
#include <algorithm>

static const Pixel kWhite{255, 255, 255};

// Создаёт изображение заданного размера и заливает цветом
Image create_blank(int width, int height, Pixel fill) {
    Image img;
    img.width = width;
    img.height = height;
    img.pixels.assign(static_cast<size_t>(width) * height, fill);
    return img;
}

static int clampi(int v, int lo, int hi) {
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

static unsigned char to_byte(double v) {
    if (v < 0.0) {
        return 0;
    }
    if (v > 255.0) {
        return 255;
    }
    return static_cast<unsigned char>(std::round(v));
}

static Pixel pixel_at_clamped(const Image& src, int x, int y) {
    x = clampi(x, 0, src.width - 1);
    y = clampi(y, 0, src.height - 1);
    return src.pixels[y * src.width + x];
}

static Pixel mix3(double r, double g, double b) {
    Pixel p;
    p.r = to_byte(r);
    p.g = to_byte(g);
    p.b = to_byte(b);
    return p;
}

static Pixel interp_nearest(const Image& src, double x, double y) {
    int ix = static_cast<int>(std::round(x));
    int iy = static_cast<int>(std::round(y));
    return pixel_at_clamped(src, ix, iy);
}

static Pixel interp_bilinear(const Image& src, double x, double y) {
    int l = static_cast<int>(std::floor(x));
    int k = static_cast<int>(std::floor(y));
    double a = x - l;
    double b = y - k;
    Pixel f00 = pixel_at_clamped(src, l, k);
    Pixel f10 = pixel_at_clamped(src, l + 1, k);
    Pixel f01 = pixel_at_clamped(src, l, k + 1);
    Pixel f11 = pixel_at_clamped(src, l + 1, k + 1);
    double w00 = (1.0 - a) * (1.0 - b);
    double w10 = a * (1.0 - b);
    double w01 = b * (1.0 - a);
    double w11 = a * b;
    return mix3(
        w00 * f00.r + w10 * f10.r + w01 * f01.r + w11 * f11.r,
        w00 * f00.g + w10 * f10.g + w01 * f01.g + w11 * f11.g,
        w00 * f00.b + w10 * f10.b + w01 * f01.b + w11 * f11.b);
}

static double lagrange4(double t, int node) {
    if (node == -1) {
        return -t * (t - 1.0) * (t - 2.0) / 6.0;
    }
    if (node == 0) {
        return (t + 1.0) * (t - 1.0) * (t - 2.0) / 2.0;
    }
    if (node == 1) {
        return -t * (t + 1.0) * (t - 2.0) / 2.0;
    }
    return t * (t + 1.0) * (t - 1.0) / 6.0;
}

static Pixel interp_bicubic(const Image& src, double x, double y) {
    int l = static_cast<int>(std::floor(x));
    int k = static_cast<int>(std::floor(y));
    double a = x - l;
    double b = y - k;

    const int ox[16] = { 0,  0,  1,  1,  0, -1,  1, -1,  0,  2, -1,  1,  2, -1,  2,  2};
    const int oy[16] = { 0,  1,  0,  1, -1,  0, -1,  1,  2,  0, -1,  2,  1,  2, -1,  2};

    double r = 0.0;
    double g = 0.0;
    double bl = 0.0;
    for (int t = 0; t < 16; ++t) {
        double c = lagrange4(a, ox[t]) * lagrange4(b, oy[t]);
        Pixel p = pixel_at_clamped(src, l + ox[t], k + oy[t]);
        r += c * p.r;
        g += c * p.g;
        bl += c * p.b;
    }
    return mix3(r, g, bl);
}

static bool mapped_inside(const Image& src, double x, double y) {
    return x >= 0.0 && y >= 0.0 && x <= src.width - 1.0 && y <= src.height - 1.0;
}

// Берёт яркость исходника в (x, y) интерполяцией nearest / bilinear / bicubic
Pixel interpolate(const Image& src, double x, double y, InterpMethod method, Pixel outside) {
    if (!mapped_inside(src, x, y)) {
        return outside;
    }
    if (method == INTERP_BILINEAR) {
        return interp_bilinear(src, x, y);
    }
    if (method == INTERP_BICUBIC) {
        return interp_bicubic(src, x, y);
    }
    return interp_nearest(src, x, y);
}

// Масштабирует изображение: i = sx*x', j = sy*y'
Image affine_scale(const Image& src, double sx, double sy, InterpMethod method) {
    int dw = std::max(1, static_cast<int>(std::round(std::fabs(sx) * src.width)));
    int dh = std::max(1, static_cast<int>(std::round(std::fabs(sy) * src.height)));
    Image dst = create_blank(dw, dh, kWhite);
    for (int j = 0; j < dh; ++j) {
        for (int i = 0; i < dw; ++i) {
            double x = i / sx;
            double y = j / sy;
            dst.pixels[j * dw + i] = interpolate(src, x, y, method, kWhite);
        }
    }
    return dst;
}

// Поворачивает изображение вокруг центра на угол в градусах
Image affine_rotate(const Image& src, double angle_deg, InterpMethod method) {
    Image dst = create_blank(src.width, src.height, kWhite);
    const double phi = angle_deg * 3.14159265358979323846 / 180.0;
    const double c = std::cos(phi);
    const double s = std::sin(phi);
    const double cx = (src.width - 1) * 0.5;
    const double cy = (src.height - 1) * 0.5;
    for (int j = 0; j < dst.height; ++j) {
        for (int i = 0; i < dst.width; ++i) {
            double dx = i - cx;
            double dy = j - cy;
            double x = c * dx + s * dy + cx;
            double y = -s * dx + c * dy + cy;
            dst.pixels[j * dst.width + i] = interpolate(src, x, y, method, kWhite);
        }
    }
    return dst;
}

// Делает скос по X вокруг центра: i = x' + sh*y', j = y'
Image affine_shear_x(const Image& src, double shear, InterpMethod method) {
    Image dst = create_blank(src.width, src.height, kWhite);
    const double cx = (src.width - 1) * 0.5;
    const double cy = (src.height - 1) * 0.5;
    for (int j = 0; j < dst.height; ++j) {
        for (int i = 0; i < dst.width; ++i) {
            double y = j;
            double x = (i - cx) - shear * (j - cy) + cx;
            dst.pixels[j * dst.width + i] = interpolate(src, x, y, method, kWhite);
        }
    }
    return dst;
}
