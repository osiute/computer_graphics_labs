#include "lab6.h"
#include "netpbm.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libraries/stb_image_write.h"

#include <cctype>
#include <stdexcept>
#include <vector>

// Загружает изображение из файла (PPM / JPG / PNG)
Image load_image(const std::string& filename) {
    auto ends_with = [](const std::string& s, const std::string& suf) {
        if (s.size() < suf.size()) {
            return false;
        }
        for (size_t k = 0; k < suf.size(); ++k) {
            char a = static_cast<char>(std::tolower(static_cast<unsigned char>(s[s.size() - suf.size() + k])));
            char b = static_cast<char>(std::tolower(static_cast<unsigned char>(suf[k])));
            if (a != b) {
                return false;
            }
        }
        return true;
    };

    if (ends_with(filename, ".ppm")) {
        return read_ppm_p3(filename);
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
    if (data == nullptr) {
        throw std::runtime_error("Не удалось открыть изображение: " + filename);
    }

    Image img;
    img.width = width;
    img.height = height;
    img.pixels.resize(static_cast<size_t>(width) * height);
    for (int i = 0; i < width * height; ++i) {
        img.pixels[i] = {data[i * 3 + 0], data[i * 3 + 1], data[i * 3 + 2]};
    }
    stbi_image_free(data);
    return img;
}

// Сохраняет изображение в PNG
void save_png(const std::string& filename, const Image& img) {
    std::vector<unsigned char> rgb(static_cast<size_t>(img.width) * img.height * 3);
    for (int i = 0; i < img.width * img.height; ++i) {
        rgb[i * 3 + 0] = img.pixels[i].r;
        rgb[i * 3 + 1] = img.pixels[i].g;
        rgb[i * 3 + 2] = img.pixels[i].b;
    }
    if (stbi_write_png(filename.c_str(), img.width, img.height, 3, rgb.data(), img.width * 3) == 0) {
        throw std::runtime_error("Не удалось записать PNG: " + filename);
    }
}
