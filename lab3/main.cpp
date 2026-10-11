#include <iostream>
#include <string>
#include <vector>

// Пока что подключаем наши будущие заголовочные файлы
// (мы создадим их чуть позже)
#include "geometry.h"
#include "dda.h"
#include "bresenham.h"
#include "svg_handler.h"
#include "canvas.h"
#include "builtin_draw.h"

const int PBM_WIDTH = 600;
const int PBM_HEIGHT = 600;

void generate_svg_and_save_segments(const std::string& filename, double radius, double initial_angle) {
    int width = PBM_WIDTH / 2;
    int height = PBM_HEIGHT / 2;
    int cx = width / 2;
    int cy = height / 2;
    auto segments = get_pentagram_segments(cx, cy, radius, initial_angle);
    save_segments_to_svg(filename, segments, width, height);
}

int main() {
    std::cout << "=== Лабораторная работа: Растеризация пентаграммы ==-\n\n" << std::endl;

    double radius = 0.0;
    double initial_angle = 0.0;
    std::string svg_filename;
    std::string pbm_filename;

    int max_allowed_radius = std::max(PBM_WIDTH, PBM_HEIGHT) / 4;
    while (true){
        std::cout << "Введите радиус описанной окружности (например, 150): ";
        std::cin >> radius;
        if (radius > 0 && radius <= max_allowed_radius) {
            break;
        } else {
            std::cout << "Радиус должен соответствовать требованию: 0 < R <= " << max_allowed_radius << "!" << std::endl;
        }
    }

    std::cout << "Введите центральный угол первой вершины в градусах (например, 0): ";
    std::cin >> initial_angle;

    std::cout << "Введите имя для SVG-файла (например, pentagram.svg): ";
    std::cin >> svg_filename;

    std::cout << "Введите имя для итогового PBM-файла (например, result.pbm): ";
    std::cin >> pbm_filename;

    std::cout << "\n[Шаг 1] Генерация и сохранение отрезков в SVG файл: " << svg_filename << "..." << std::endl;
    generate_svg_and_save_segments(svg_filename, radius, initial_angle);

    std::cout << "[Шаг 2] Инициализация холста..." << std::endl;
    auto canvas = create_canvas(PBM_WIDTH, PBM_HEIGHT);
    int cx = PBM_WIDTH / 2;
    int cy = PBM_HEIGHT / 2;
    draw_grid(canvas, cx, cy);

    std::cout << "[Шаг 3] Растеризация (4 пентаграммы 4 разными алгоритмами)..." << std::endl;
    int x_offset = 0, y_offset = 0;

    x_offset = cx;
    y_offset = cy;
    builtin_draw_segments(canvas, svg_filename, x_offset, y_offset);

    std::cout << "[Шаг 4] Сохранение итогового растра в PBM файл: " << pbm_filename << "..." << std::endl;
    save_canvas_to_pbm(pbm_filename, canvas);

    std::cout << "\nПрограмма успешно завершена!" << std::endl;
    return 0;
}