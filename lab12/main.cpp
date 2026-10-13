#include <iostream>
#include <string>
#include <vector>
#include "image.h"
#include "netpbm.h"
#include "geometry.h"
#include "surfaces.h"

int main() {
    // Устанавливаем русскую локаль для корректного вывода в консоль
    std::setlocale(LC_ALL, "Russian");
    
    std::cout << "==================================================" << std::endl;
    std::cout << " Лабораторная работа #12: Поверхности (Безье / Ду-Сабин) " << std::endl;
    std::cout << "==================================================" << std::endl;

    int choice = 0;
    std::cout << "\nВыберите режим работы:\n"
              << " 1 - Поверхность Безье (по сетке контрольных точек)\n"
              << " 2 - Поверхность Ду-Сабина (итеративное подразделение сетки)\n"
              << "Ваш выбор (1 или 2): ";
    std::cin >> choice;

    int width = 800;
    int height = 600;
    Image img;
    img.width = width;
    img.height = height;
    img.pixels.assign(width * height, Pixel{255, 255, 255}); // Заливка белым фоном

    std::string filename;
    std::cout << "Введите имя выходного PPM-файла (например, result.ppm): ";
    std::cin >> filename;

    if (choice == 1) {
        // Запрос параметра разбиения у пользователя
        int resolution = 15;
        std::cout << "Введите параметр разбиения (например, 15 или 20): ";
        std::cin >> resolution;
        if (resolution < 2) resolution = 2;


        // Задаем контрольные точки сразу с пространственным наклоном (эффект поворота)
        // Левая часть уходит вглубь (-Z), правая выдвигается вперед (+Z)
        std::vector<std::vector<Point3D>> control_grid = {
            { {-140, -120,  -50}, {-60, -120,  -20}, {20, -120,   20}, {100, -120,   50} },
            { {-140,  -40,  -20}, {-60,  -40,   50}, {20,  -40,   90}, {100,  -40,  120} },
            { {-140,   40,  -20}, {-60,   40,   50}, {20,   40,   90}, {100,   40,  120} },
            { {-140,  120,  -50}, {-60,  120,  -20}, {20,  120,   20}, {100,  120,   50} }
        };

        // Генерируем поверхность Безье
        Mesh surface_mesh = generate_bezier_surface(control_grid, resolution, resolution);

        // Рисуем саму поверхность (зеленая сетка)
        // Цвет сетки: темно-зеленый (0, 130, 0), цвет вершин/точек: красный (200, 30, 30)
        draw_mesh(img, surface_mesh, 450.0, Pixel{0, 130, 0}, Pixel{200, 30, 30});

        // Также отдельно отобразим контрольные точки и каркас для наглядности (пункт 6 методички)
        Mesh control_mesh;
        for (const auto& row : control_grid) {
            for (const auto& pt : row) {
                control_mesh.vertices.push_back(pt);
            }
        }
        // Создаем продольные и поперечные линии каркаса контрольных точек
        for (int i = 0; i < 4; ++i) {
            // Ряды по горизонтали
            control_mesh.faces.push_back({{i*4, i*4+1, i*4+2, i*4+3}});
            // Ряды по вертикали
            control_mesh.faces.push_back({{i, i+4, i+8, i+12}});
        }
        
        // Рисуем контрольный каркас тонкими линиями
        draw_mesh(img, control_mesh, 450.0, Pixel{100, 150, 255}, Pixel{220, 20, 20});

    } else if (choice == 2) {
        std::cout << "\n[Заглушка] Выбрано подразделение Ду-Сабина..." << std::endl;
        // Здесь в будущем вызовем алгоритм Ду-Сабина

        Mesh test_mesh;
        // Тестовый куб
        double s = 80.0;
        test_mesh.vertices = {
            {-s, -s, -s}, {s, -s, -s}, {s, s, -s}, {-s, s, -s},
            {-s, -s,  s}, {s, -s,  s}, {s, s,  s}, {-s, s,  s}
        };
        test_mesh.faces = {
            {{0, 3, 2, 1}}, {{4, 5, 6, 7}},
            {{0, 1, 5, 4}}, {{2, 3, 7, 6}},
            {{0, 4, 7, 3}}, {{1, 2, 6, 5}}
        };

        draw_mesh(img, test_mesh, 350.0, Pixel{0, 130, 0}, Pixel{200, 30, 30});
    } else {
        std::cout << "Неверный выбор! Программа завершена." << std::endl;
        return 1;
    }

    // Сохраняем результат в PPM-файл
    write_ppm_p3(filename, img);
    std::cout << "Успешно! Результат сохранен в файл: " << filename << std::endl;

    return 0;
}