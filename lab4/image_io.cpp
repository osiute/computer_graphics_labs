#include "lab4.h"
#include "netpbm.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb_image.h"

#include <cctype>
#include <stdexcept>

// Загружает изображение из файла (PPM / JPG / PNG)
Image load_image(const std::string& filename) {
    auto ends_with = [](const std::string& s, const std::string& suf) {
        if (s.size() < suf.size()) {
            return false;
        }
        for (size_t i = 0; i < suf.size(); ++i) {
            char a = static_cast<char>(std::tolower(static_cast<unsigned char>(s[s.size() - suf.size() + i])));
            char b = static_cast<char>(std::tolower(static_cast<unsigned char>(suf[i])));
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
