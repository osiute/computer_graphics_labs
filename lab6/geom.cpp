#include "lab6.h"
#include <cmath>

static const Pixel kWhite{255, 255, 255};

static double math_y(double pixel_y, int height) {
    return static_cast<double>(height - 1) - pixel_y;
}

static double pixel_y(double math_j, int height) {
    return static_cast<double>(height - 1) - math_j;
}

// Создаёт изображение заданного размера и заливает цветом
Image create_blank(int width, int height, Pixel fill) {
    Image img;
    img.width = width;
    img.height = height;
    img.pixels.assign(static_cast<size_t>(width) * height, fill);
    return img;
}

// Берёт ближайший пиксель исходника в точке (x, y)
Pixel sample_nearest(const Image& src, double x, double y, Pixel outside) {
    int ix = static_cast<int>(std::round(x));
    int iy = static_cast<int>(std::round(y));
    if (ix < 0 || iy < 0 || ix >= src.width || iy >= src.height) {
        return outside;
    }
    return src.pixels[iy * src.width + ix];
}

// Ставит пиксель в точку (x, y), если она внутри кадра
void put_pixel(Image& img, int x, int y, Pixel color) {
    if (x < 0 || y < 0 || x >= img.width || y >= img.height) {
        return;
    }
    img.pixels[y * img.width + x] = color;
}

// Прямое аффинное: i = a*x' + c, j = e*y' + f (начало — левый нижний угол)
Image affine_forward(const Image& src, double a, double e, double c, double f) {
    Image dst = create_blank(src.width, src.height, kWhite);
    for (int y = 0; y < src.height; ++y) {
        for (int x = 0; x < src.width; ++x) {
            double i = a * x + c;
            double j = e * math_y(y, src.height) + f;
            put_pixel(dst, static_cast<int>(std::round(i)),
                      static_cast<int>(std::round(pixel_y(j, dst.height))),
                      src.pixels[y * src.width + x]);
        }
    }
    return dst;
}

// Обратное аффинное: x' = (i-c)/a, y' = (j-f)/e (без дыр)
Image affine_inverse(const Image& src, double a, double e, double c, double f) {
    Image dst = create_blank(src.width, src.height, kWhite);
    for (int j_pix = 0; j_pix < dst.height; ++j_pix) {
        for (int i = 0; i < dst.width; ++i) {
            double x_src = (i - c) / a;
            double y_src_math = (math_y(j_pix, dst.height) - f) / e;
            double y_src = pixel_y(y_src_math, src.height);
            dst.pixels[j_pix * dst.width + i] = sample_nearest(src, x_src, y_src, kWhite);
        }
    }
    return dst;
}

// Восстанавливает исходник обратным аффинным преобразованием
Image affine_restore(const Image& transformed, double a, double e, double c, double f) {
    Image dst = create_blank(transformed.width, transformed.height, kWhite);
    for (int y = 0; y < dst.height; ++y) {
        for (int x = 0; x < dst.width; ++x) {
            double i = a * x + c;
            double j = e * math_y(y, dst.height) + f;
            dst.pixels[y * dst.width + x] = sample_nearest(
                transformed, i, pixel_y(j, transformed.height), kWhite);
        }
    }
    return dst;
}

// Преобразует координаты по закону i = 2^{x'}, j = y'
Image functional_pow2_x(const Image& src) {
    Image dst = create_blank(src.width, src.height, kWhite);
    const double ln2 = std::log(2.0);
    for (int j = 0; j < dst.height; ++j) {
        for (int i = 1; i < dst.width; ++i) {
            double x_src = std::log(static_cast<double>(i)) / ln2;
            dst.pixels[j * dst.width + i] = sample_nearest(src, x_src, static_cast<double>(j), kWhite);
        }
    }
    return dst;
}
