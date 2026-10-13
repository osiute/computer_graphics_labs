#include <iostream>
#include <string>
#include <vector>
#include "image.h"
#include "netpbm.h"
#include "geometry.h"

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
        std::cout << "\n[Заглушка] Выбрана генерация поверхности Безье..." << std::endl;
        // Здесь в будущем вызовем функцию генерации Безье
        
        // Тестовая демонстрация отрисовки сетки/геометрии:
        Mesh test_mesh;
        test_mesh.vertices = {{-50, -50, 0}, {50, -50, 0}, {50, 50, 0}, {-50, 50, 0}};
        test_mesh.faces = {{{0, 1, 2, 3}}};
        
        // Рисуем рамку (синие ребра, красные вершины)
        draw_mesh(img, test_mesh, 350.0, Pixel{30, 90, 180}, Pixel{200, 30, 30});

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