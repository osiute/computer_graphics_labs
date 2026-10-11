#include "filters.h"
#include <algorithm>
#include <cmath>

void apply_variant_filter(Image& img) {
    for (auto& p : img.pixels) {
        HSV hsv = rgb_to_hsv(p);
        
        // Если яркость (V) меньше 50% (0.5), увеличиваем насыщенность (S) на 30%
        if (hsv.v < 0.5) {
            hsv.s = std::min(1.0, hsv.s * 1.3);
        }
        
        p = hsv_to_rgb(hsv);
    }
}

Image blend_images(const Image& base, const Image& overlay, double alpha) {
    int w = std::min(base.width, overlay.width);
    int h = std::min(base.height, overlay.height);

    Image result;
    result.width = w;
    result.height = h;
    result.pixels.resize(w * h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            Pixel b = base.pixels[y * base.width + x];
            Pixel o = overlay.pixels[y * overlay.width + x];

            // Формула линейной интерполяции для каждого канала RGB
            Pixel res;
            res.r = static_cast<unsigned char>(alpha * o.r + (1.0 - alpha) * b.r);
            res.g = static_cast<unsigned char>(alpha * o.g + (1.0 - alpha) * b.g);
            res.b = static_cast<unsigned char>(alpha * o.b + (1.0 - alpha) * b.b);
            
            result.pixels[idx] = res;
        }
    }
    return result;
}