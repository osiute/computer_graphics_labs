#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
#include "image.h"
#include "netpbm.h"
#include "lab4.h"

using CircleFn = void (*)(Image&, int, int, int, Pixel);

// Рисует пятиугольник, описанную и вписанную окружности одним алгоритмом
static void draw_variant_in_quadrant(Image& img, int cx, int cy, int R, int r, CircleFn circle) {
    const Pixel pentagon_color{20, 20, 20};
    const Pixel circum_color{200, 40, 40};
    const Pixel in_color{40, 80, 220};
    draw_regular_pentagon(img, cx, cy, R, pentagon_color);
    circle(img, cx, cy, R, circum_color);
    circle(img, cx, cy, r, in_color);
}

// Загружает картинку, рисует 4 варианта окружностей и сохраняет PPM
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Лабораторная работа #4: Растеризация окружностей (вариант 2) ===" << std::endl;
    std::cout << "Фигура: правильный пятиугольник по стороне a." << std::endl;
    std::cout << "Рисуем вписанную и описанную окружности 4 алгоритмами." << std::endl;

    std::string input_file;
    std::string output_file;
    double side = 0.0;

    std::cout << "Введите путь к исходному изображению (ppm/jpg/png): ";
    std::cin >> input_file;
    std::cout << "Введите длину стороны пятиугольника a (например, 120): ";
    std::cin >> side;
    std::cout << "Введите путь для сохранения результата (.ppm): ";
    std::cin >> output_file;

    if (side <= 0.0) {
        std::cerr << "Сторона должна быть положительной." << std::endl;
        return 1;
    }

    try {
        std::cout << "\n[1/4] Чтение исходного изображения..." << std::endl;
        Image img = load_image(input_file);

        int qw = img.width / 2;
        int qh = img.height / 2;
        int max_R = std::min(qw, qh) / 2 - 8;
        if (max_R < 10) {
            throw std::runtime_error("Изображение слишком маленькое для окружностей.");
        }

        double R_d = circumradius_from_side(side, 5);
        double r_d = inradius_from_side(side, 5);
        if (R_d > max_R) {
            double k = max_R / R_d;
            R_d *= k;
            r_d *= k;
            std::cout << "  сторона слишком большая, масштабировал под квадрант." << std::endl;
        }
        int R = std::max(1, static_cast<int>(std::round(R_d)));
        int r = std::max(1, static_cast<int>(std::round(r_d)));

        std::cout << "[2/4] Радиусы: описанная R = " << R << ", вписанная r = " << r << std::endl;
        std::cout << "[3/4] Растеризация 4 алгоритмами..." << std::endl;

        int c0x = qw / 2;
        int c0y = qh / 2;
        int c1x = qw + qw / 2;
        int c1y = qh / 2;
        int c2x = qw / 2;
        int c2y = qh + qh / 2;
        int c3x = qw + qw / 2;
        int c3y = qh + qh / 2;

        std::cout << "  левый верх: уравнение окружности" << std::endl;
        draw_variant_in_quadrant(img, c0x, c0y, R, r, draw_circle_equation);

        std::cout << "  правый верх: параметрическое уравнение" << std::endl;
        draw_variant_in_quadrant(img, c1x, c1y, R, r, draw_circle_parametric);

        std::cout << "  левый низ: Брезенхем" << std::endl;
        draw_variant_in_quadrant(img, c2x, c2y, R, r, draw_circle_bresenham);

        std::cout << "  правый низ: CImg::draw_circle" << std::endl;
        draw_variant_in_quadrant(img, c3x, c3y, R, r, draw_circle_cimg);

        draw_split(img, Pixel{0, 0, 0});

        std::cout << "[4/4] Сохранение PPM через write_ppm_p3..." << std::endl;
        write_ppm_p3(output_file, img);
        std::cout << "\nУспех! Результат записан." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
