#include <iostream>
#include <fstream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Вспомогательная функция для установки цвета пикселя
void set_pixel(unsigned char* img, int width, int height, int channels, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;

    int index = (y * width + x) * channels;
    img[index + 0] = r;
    img[index + 1] = g;
    img[index + 2] = b;
}

// Функция для рисования заметной точки
void draw_point(unsigned char* img, int width, int height, int channels, int cx, int cy, unsigned char r, unsigned char g, unsigned char b, int size = 5) {
    int half = size / 2;
    for (int dy = -half; dy <= half; ++dy) {
        for (int dx = -half; dx <= half; ++dx) {
            set_pixel(img, width, height, channels, cx + dx, cy + dy, r, g, b);
        }
    }
}

// Функция сохранения изображения в текстовый формат PPM (P3)
void save_as_ascii_ppm(const char* filename, int width, int height, int channels, const unsigned char* img) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Ошибка создания файла " << filename << std::endl;
        return;
    }

    out << "P3\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * channels;
            int r = img[index + 0];
            int g = img[index + 1];
            int b = img[index + 2];
            out << r << " " << g << " " << b << " ";
        }
        out << "\n";
    }
    out.close();
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Использование: " << argv[0] << " <input_file> <output_file.jpg> <output_file.ppm> [point_size]" << std::endl;
        return 1;
    }

    const char* input_filename = argv[1];
    const char* output_filename_jpg = argv[2];
    const char* output_filename_ppm = argv[3];
    int point_size = (argc >= 5) ? std::stoi(argv[4]) : 1;

    int width, height, channels;
    unsigned char* img = stbi_load(input_filename, &width, &height, &channels, 0);

    if (img == nullptr) {
        std::cerr << "Ошибка загрузки файла " << input_filename << std::endl;
        return 1;
    }

    // 1. Точка (0, 0, 255) — Синяя в верхнем левом углу
    draw_point(img, width, height, channels, 0, 0, 0, 0, 255, point_size);

    // 2. Точка (255, 255, 0) — Желтая в центре верхней строки
    draw_point(img, width, height, channels, width / 2, 0, 255, 255, 0, point_size);

    // 3. Точка (255, 0, 255) — Пурпурная в центре изображения
    draw_point(img, width, height, channels, width / 2, height / 2, 255, 0, 255, point_size);

    // Сохранение в формате jpg
    stbi_write_jpg(output_filename_jpg, width, height, channels, img, 100);
    std::cout << "Изображение в формате jpg сохранено в: " << output_filename_jpg << std::endl;

    // Сохранение в формате ppm (дополнительное задание указывает, что изображение должно сохраняться в формате pbm,
    // однако в данном формате хранятся монохромные однобитовые изображения. ppm принадлежит тому же семейство, однако
    // работает с цветными RGB изображениями).
    save_as_ascii_ppm(output_filename_ppm, width, height, channels, img);
    std::cout << "Изображение в формате ppm сохранено в: " << output_filename_ppm << std::endl;

    stbi_image_free(img);
    return 0;
}