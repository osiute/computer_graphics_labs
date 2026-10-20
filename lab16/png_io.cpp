#include "png_io.h"

#include <fstream>
#include <stdexcept>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace lab16 {
namespace {
void write_bytes(void* context, void* data, int size) {
    static_cast<std::ofstream*>(context)->write(static_cast<const char*>(data), size);
}
}

void save_png(const std::filesystem::path& path, const Image& image) {
    if (image.width <= 0 || image.height <= 0 || image.width > 4096 || image.height > 4096
        || image.pixels.size() != static_cast<std::size_t>(image.width) * image.height) {
        throw std::runtime_error("Некорректное изображение для записи PNG.");
    }
    std::vector<unsigned char> rgb;
    rgb.reserve(image.pixels.size() * 3);
    for (const auto& p : image.pixels) {
        rgb.push_back(p.r);
        rgb.push_back(p.g);
        rgb.push_back(p.b);
    }
    std::ofstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Не удалось создать PNG: " + path.u8string());
    const int ok = stbi_write_png_to_func(write_bytes, &file, image.width, image.height, 3,
                                         rgb.data(), image.width * 3);
    file.flush();
    if (!ok || !file) throw std::runtime_error("Ошибка записи PNG: " + path.u8string());
    file.close();
    if (!file) throw std::runtime_error("Ошибка закрытия PNG: " + path.u8string());
}
} // namespace lab16
