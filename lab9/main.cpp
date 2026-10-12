#include <iostream>
#include <vector>
#include <string>
#include "../common_utils/image.h"
#include "../common_utils/netpbm.h"
#include "clipper.h"

int main() {
    std::setlocale(LC_ALL, "Russian");
    std::cout << "=== Лабораторная работа #9: Отсечение отрезков (Алгоритм Коэна-Сазерленда) ===" << std::endl;

    // 1. Задаем размеры холста
    int width = 800;
    int height = 600;
    std::cout << "Введите ширину и высоту холста (например, 800 600): ";
    std::cin >> width >> height;

    // Создаем изображение с белым фоном, используя структуру Image из image.h[cite: 1]
    Image img;
    img.width = width;
    img.height = height;
    img.pixels.assign(width * height, Pixel{255, 255, 255});

    // 2. Задаем окно отсечения
    Rect rect;
    std::cout << "Введите координаты окна отсечения (xmin ymin xmax ymax): ";
    std::cin >> rect.xmin >> rect.ymin >> rect.xmax >> rect.ymax;

    // 3. Задаем набор отрезков
    int num_lines = 0;
    std::cout << "Введите количество отрезков: ";
    std::cin >> num_lines;

    std::vector<Line> lines(num_lines);
    for (int i = 0; i < num_lines; ++i) {
        std::cout << "Отрезок " << (i + 1) << " (x1 y1 x2 y2): ";
        std::cin >> lines[i].p1.x >> lines[i].p1.y >> lines[i].p2.x >> lines[i].p2.y;
    }

    std::string output_file;
    std::cout << "Введите путь для сохранения результата (.ppm): ";
    std::cin >> output_file;

    // Опционально: рисуем исходные отрезки тонкими/светлыми линиями (серо-голубыми) для сравнения
    Pixel original_color = {200, 200, 200};
    for (const auto& line : lines) {
        draw_line(img, line.p1.x, line.p1.y, line.p2.x, line.p2.y, original_color);
    }

    // Рисуем рамку окна отсечения (красный цвет)
    Pixel window_color = {255, 0, 0};
    draw_rect(img, rect, window_color);

    // Применяем алгоритм Коэна-Сазерленда и рисуем видимые части отрезков (зеленый цвет)
    Pixel clipped_color = {0, 180, 0};
    int visible_count = 0;

    for (const auto& line : lines) {
        int x0 = line.p1.x;
        int y0 = line.p1.y;
        int x1 = line.p2.x;
        int y1 = line.p2.y;

        // Если отрезок виден (хотя бы частично), рисуем его оставшуюся часть
        if (cohen_sutherland_clip(x0, y0, x1, y1, rect)) {
            draw_line(img, x0, y0, x1, y1, clipped_color);
            visible_count++;
        }
    }

    // Сохраняем результат в формат PPM P3 с помощью функции из netpbm.h[cite: 2]
    write_ppm_p3(output_file, img);

    std::cout << "\nУспех! Обработано отрезков: " << num_lines 
              << ", видимых после отсечения: " << visible_count 
              << ".\nРезультат сохранен в файл: " << output_file << std::endl;

    return 0;
}