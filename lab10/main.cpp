#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "image.h"
#include "netpbm.h"
#include "curves.h"

// Алгоритм Брезенхема для рисования линий
void draw_line(Image& img, int x0, int y0, int x1, int y1, const Pixel& color) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && x0 < img.width && y0 >= 0 && y0 < img.height) {
            img.pixels[y0 * img.width + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Отрисовка контрольной точки (кружок)
void draw_point(Image& img, int x, int y, int radius, const Pixel& color) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= radius * radius) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < img.width && ny >= 0 && ny < img.height) {
                    img.pixels[ny * img.width + nx] = color;
                }
            }
        }
    }
}

int main() {
    std::setlocale(LC_ALL, "Russian");
    std::cout << "=== Лабораторная работа #10: Построение кривых ===" << std::endl;

    int width = 800;
    int height = 600;
    Image img;
    img.width = width;
    img.height = height;
    img.pixels.assign(width * height, Pixel{255, 255, 255}); // Белый фон

    int choice;
    std::cout << "Выберите тип кривой:\n"
              << " 1 - Квадратичная кривая Безье (3 точки)\n"
              << " 2 - Кубическая кривая Безье (4 точки)\n"
              << " 3 - Кривая Чайкина (произвольный полигон)\n"
              << "Ваш выбор: ";
    std::cin >> choice;

    std::vector<Point2D> control_points;

    if (choice == 1) {
        std::cout << "Введите 3 контрольные точки (x y в диапазоне 0-800 и 0-600):\n";
        for (int i = 0; i < 3; ++i) {
            double x, y;
            std::cout << "Точка " << i + 1 << " (x y): ";
            std::cin >> x >> y;
            control_points.push_back({x, y});
        }
    } else if (choice == 2) {
        std::cout << "Введите 4 контрольные точки (x y):\n";
        for (int i = 0; i < 4; ++i) {
            double x, y;
            std::cout << "Точка " << i + 1 << " (x y): ";
            std::cin >> x >> y;
            control_points.push_back({x, y});
        }
    } else {
        int n;
        std::cout << "Введите количество точек ломаной для Чайкина (например, 4-6): ";
        std::cin >> n;
        std::cout << "Введите " << n << " точек (x y):\n";
        for (int i = 0; i < n; ++i) {
            double x, y;
            std::cout << "Точка " << i + 1 << " (x y): ";
            std::cin >> x >> y;
            control_points.push_back({x, y});
        }
    }

    std::string filename;
    std::cout << "Введите имя файла для сохранения (.ppm): ";
    std::cin >> filename;

    // Рисуем исходный контрольный полигон (пунктиром или тонкими серыми линиями)
    Pixel poly_color = {180, 180, 180};
    for (size_t i = 0; i < control_points.size() - 1; ++i) {
        draw_line(img, static_cast<int>(control_points[i].x), static_cast<int>(control_points[i].y),
                       static_cast<int>(control_points[i+1].x), static_cast<int>(control_points[i+1].y), poly_color);
    }

    // Рисуем сами контрольные точки (синие кружки)
    Pixel pt_color = {30, 80, 180};
    for (const auto& pt : control_points) {
        draw_point(img, static_cast<int>(pt.x), static_cast<int>(pt.y), 5, pt_color);
    }

    // Генерация и отрисовка кривой
    std::vector<Point2D> curve_points;
    if (choice == 1) {
        curve_points = generate_quadratic_bezier(control_points, 200);
    } else if (choice == 2) {
        curve_points = generate_cubic_bezier(control_points, 200);
    } else {
        int iter;
        std::cout << "Введите количество итераций уточнения для Чайкина (например, 3-5): ";
        std::cin >> iter;
        curve_points = generate_chaikin(control_points, iter);
    }

    // Отрисовка результирующей кривой (красный цвет)
    Pixel curve_color = {220, 20, 20};
    for (size_t i = 0; i < curve_points.size() - 1; ++i) {
        draw_line(img, static_cast<int>(curve_points[i].x), static_cast<int>(curve_points[i].y),
                       static_cast<int>(curve_points[i+1].x), static_cast<int>(curve_points[i+1].y), curve_color);
    }

    write_ppm_p3(filename, img);
    std::cout << "Успех! Кривая построена и сохранена в файл: " << filename << std::endl;

    return 0;
}