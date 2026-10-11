#include "filters_lab7.h"
#include <cmath>
#include <algorithm>
#include <vector>

// Вспомогательная функция для генерации двумерного ядра Гаусса
static std::vector<std::vector<double>> generate_gaussian_kernel(int radius, double sigma) {
    int size = 2 * radius + 1;
    std::vector<std::vector<double>> kernel(size, std::vector<double>(size));
    double sum = 0.0;
    
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            double val = std::exp(-(x * x + y * y) / (2 * sigma * sigma));
            kernel[y + radius][x + radius] = val;
            sum += val;
        }
    }
    
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            kernel[y][x] /= sum;
        }
    }
    return kernel;
}

// Универсальная функция применения матричной свёртки к пикселю (cx, cy)
static Pixel apply_kernel_at(const Image& img, int cx, int cy, const std::vector<std::vector<double>>& kernel) {
    int k_size = kernel.size();
    int radius = k_size / 2;
    
    double r_sum = 0.0;
    double g_sum = 0.0;
    double b_sum = 0.0;
    
    for (int ky = 0; ky < k_size; ++ky) {
        for (int kx = 0; kx < k_size; ++kx) {
            int px = cx + (kx - radius);
            int py = cy + (ky - radius);
            
            px = std::clamp(px, 0, img.width - 1);
            py = std::clamp(py, 0, img.height - 1);
            
            const Pixel& p = img.pixels[py * img.width + px];
            double weight = kernel[ky][kx];
            
            r_sum += p.r * weight;
            g_sum += p.g * weight;
            b_sum += p.b * weight;
        }
    }
    
    return Pixel{
        static_cast<unsigned char>(std::clamp(std::round(r_sum), 0.0, 255.0)),
        static_cast<unsigned char>(std::clamp(std::round(g_sum), 0.0, 255.0)),
        static_cast<unsigned char>(std::clamp(std::round(b_sum), 0.0, 255.0))
    };
}

Image apply_gaussian_blur_outside_circle(const Image& img, double radius_circle, double sigma) {
    int kernel_radius = 2; // Размер ядра 5x5
    auto kernel = generate_gaussian_kernel(kernel_radius, sigma);
    
    Image result = img; // Создаем копию исходного изображения
    double cx = img.width / 2.0;
    double cy = img.height / 2.0;
    
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            double dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
            
            if (dist > radius_circle) {
                result.pixels[y * img.width + x] = apply_kernel_at(img, x, y, kernel);
            }
        }
    }
    return result;
}

Image apply_sharpen_above_brightness(const Image& img, double threshold) {
    std::vector<std::vector<double>> sharpen_kernel = {
        { 0.0, -1.0,  0.0},
        {-1.0,  5.0, -1.0},
        { 0.0, -1.0,  0.0}
    };
    
    Image result = img; // Создаем копию исходного изображения
    
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            int idx = y * img.width + x;
            Pixel p = img.pixels[idx];
            
            HSV hsv = rgb_to_hsv(p);
            
            if (hsv.v > threshold) {
                result.pixels[idx] = apply_kernel_at(img, x, y, sharpen_kernel);
            }
        }
    }
    return result;
}