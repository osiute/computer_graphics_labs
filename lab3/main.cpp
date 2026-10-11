#include <iostream>
#include <string>
#include <vector>

// Пока что подключаем наши будущие заголовочные файлы
// (мы создадим их чуть позже)
#include "geometry.h"
#include "dda.h"
#include "bresenham.h"

int main() {
    std::cout << "=== Лабораторная работа: Растеризация пентаграммы ==-\n\n" << std::endl;

    // 1. Интерактивный ввод параметров от пользователя
    double radius = 0.0;
    double initial_angle = 0.0;
    std::string svg_filename;
    std::string pbm_filename;

    while (true){
        std::cout << "Введите радиус описанной окружности (например, 150): ";
        std::cin >> radius;
        if (radius > 0 && radius <= 200) {
            break;
        } else {
            std::cout << "Радиус должен соответствовать требованию: 0 < R <= 200!" << std::endl;
        }
    }

    std::cout << "Введите центральный угол первой вершины в градусах (например, 0): ";
    std::cin >> initial_angle;

    std::cout << "Введите имя для SVG-файла (например, pentagram.svg): ";
    std::cin >> svg_filename;

    std::cout << "Введите имя для итогового PBM-файла (например, result.pbm): ";
    std::cin >> pbm_filename;

    std::cout << "\n[Шаг 1] Генерация и сохранение отрезков в SVG файл: " << svg_filename << "..." << std::endl;
    // Заглушка: здесь будет функция сохранения в SVG
    // generate_and_save_svg(svg_filename, radius, initial_angle);

    std::cout << "[Шаг 2] Чтение отрезков из SVG файла..." << std::endl;
    // Заглушка: здесь будет функция парсинга SVG
    // std::vector<Segment> segments = read_svg_segments(svg_filename);

    std::cout << "[Шаг 3] Инициализация холста и растеризация (4 алгоритма в 4 квадрантах)..." << std::endl;
    // Заглушка: здесь будет создание холста и вызовы ЦДА, Брезенхема и т.д.

    std::cout << "[Шаг 4] Сохранение итогового растра в PBM файл: " << pbm_filename << "..." << std::endl;
    // Заглушка: сохранение матрицы пикселей в формат PBM (P1)

    std::cout << "\nПрограмма успешно завершена!" << std::endl;
    return 0;
}