#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "image.h"
#include "netpbm.h"
#include "geometry.h"
#include "parser.h"
#include "clipper.h"
#include "filler.h"

void draw_polygon_outline(Image& img, const std::vector<Point2D>& polygon, const Pixel& color) {
    if (polygon.size() < 2) return;
    size_t n = polygon.size();
    for (size_t i = 0; i < n; ++i) {
        int x0 = static_cast<int>(std::round(polygon[i].x));
        int y0 = static_cast<int>(std::round(polygon[i].y));
        int x1 = static_cast<int>(std::round(polygon[(i + 1) % n].x));
        int y1 = static_cast<int>(std::round(polygon[(i + 1) % n].y));
        draw_line(img, x0, y0, x1, y1, color);
    }
}

int main() {
    std::string input_filename;
    std::string output_filename;

    std::cout << "=== Лабораторная работа №13: Отсечение и закрашивание многоугольников ===" << std::endl;
    std::cout << "Введите имя входного файла (например, input.txt): ";
    std::cin >> input_filename;

    scene_data scene;
    if (!load_scene_from_file(input_filename, scene)) {
        std::cerr << "Не удалось загрузить данные из файла: " << input_filename << std::endl;
        return 1;
    }

    std::cout << "Введите имя выходного PPM-файла (например, result.ppm): ";
    std::cin >> output_filename;

    // Создаем холст на основе размеров из файла
    Image img;
    img.width = scene.canvas_width;
    img.height = scene.canvas_height;
    img.pixels.assign(img.width * img.height, Pixel{255, 255, 255});

    // 1. Отсечение многоугольника выпуклым окном (Сазерленд — Ходжмен)
    std::vector<Point2D> clipped_polygon = sutherland_hodgman_clip(scene.subject_polygon, scene.clip_window);

    // 2. Растровое закрашивание полученной области (мягкий красный цвет)
    Pixel fill_color{255, 180, 180};
    if (!clipped_polygon.empty()) {
        rasterize_polygon_fill(img, clipped_polygon, fill_color);
    }

    // 3. Отрисовка контуров с цветовым кодированием:
    // - Исходный многоугольник: чёрный {0, 0, 0}
    // - Отсекающее окно: синий {0, 0, 255}
    Pixel subject_color{0, 0, 0};
    Pixel window_color{0, 0, 255};

    draw_polygon_outline(img, scene.subject_polygon, subject_color);
    draw_polygon_outline(img, scene.clip_window, window_color);

    // 4. Сохранение результата в ASCII PPM (P3)
    try {
        write_ppm_p3(output_filename, img);
        std::cout << "Успешно! Результат сохранен в файл: " << output_filename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка при сохранении файла: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}