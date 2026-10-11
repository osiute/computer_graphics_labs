#include "netpbm.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

Image read_ppm_p3(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл: " + filename);
    }

    std::string magic;
    file >> magic;
    if (magic != "P3") {
        throw std::runtime_error("Поддерживается только формат ASCII PPM (P3)!");
    }

    int width, height, max_val;
    file >> width >> height >> max_val;

    Image img;
    img.width = width;
    img.height = height;
    img.pixels.resize(width * height);

    for (int i = 0; i < width * height; ++i) {
        int r, g, b;
        file >> r >> g >> b;
        img.pixels[i] = {
            static_cast<unsigned char>(r),
            static_cast<unsigned char>(g),
            static_cast<unsigned char>(b)
        };
    }
    return img;
}

void write_ppm_p3(const std::string& filename, const Image& img) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось создать файл для записи: " + filename);
    }

    file << "P3\n";
    file << img.width << " " << img.height << "\n";
    file << "255\n";

    for (const auto& p : img.pixels) {
        file << static_cast<int>(p.r) << " "
             << static_cast<int>(p.g) << " "
             << static_cast<int>(p.b) << "\n";
    }
}