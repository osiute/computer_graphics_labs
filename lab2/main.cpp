#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
#include "image.h"
#include "netpbm.h"
#include "lab2_ops.h"

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Лабораторная работа #2: Формирование изображений (вариант 2) ===" << std::endl;
    std::cout << "Фрагмент: повёрнутый квадрат (ромб) слева по центру исходника." << std::endl;
    std::cout << "Куда: левый нижний угол нового изображения." << std::endl;
    std::cout << "График: cos(x)." << std::endl;

    int new_width = 0;
    int new_height = 0;
    std::string input_file;
    std::string output_file;

    std::cout << "Введите ширину нового изображения (например, 800): ";
    std::cin >> new_width;
    std::cout << "Введите высоту нового изображения (например, 600): ";
    std::cin >> new_height;
    std::cout << "Введите путь к исходному файлу (ppm/jpg/png): ";
    std::cin >> input_file;
    std::cout << "Введите путь для сохранения результата (.ppm): ";
    std::cin >> output_file;

    if (new_width <= 0 || new_height <= 0) {
        std::cerr << "Размеры нового изображения должны быть положительными." << std::endl;
        return 1;
    }

    try {
        std::cout << "\n[1/5] Создание нового изображения..." << std::endl;
        Image canvas = create_blank(new_width, new_height, Pixel{255, 255, 255});

        std::cout << "[2/5] Чтение исходного изображения..." << std::endl;
        Image src = load_image(input_file);

        const int half_side = std::max(24, std::min({src.width, src.height, new_width, new_height}) / 10);
        const int src_cx = src.width / 4;
        const int src_cy = src.height / 2;
        const int margin = 18;
        const int dst_cx = margin + half_side;
        const int dst_cy = new_height - margin - half_side;
        const double angle_deg = 38.0;

        std::cout << "[3/5] Копирование повёрнутого квадрата попиксельно..." << std::endl;
        std::cout << "  источник: центр (" << src_cx << ", " << src_cy << "), полусторона " << half_side << std::endl;
        std::cout << "  назначение: центр (" << dst_cx << ", " << dst_cy << "), угол " << angle_deg << " deg" << std::endl;
        copy_rotated_square(src, canvas, src_cx, src_cy, half_side, dst_cx, dst_cy, angle_deg);

        std::cout << "[4/5] Оси со стрелками и график cos(x)..." << std::endl;
        int ox = new_width * 28 / 100;
        int oy = new_height * 64 / 100;
        draw_axes_with_arrows(canvas, ox, oy, Pixel{0, 0, 0});

        double px_per_unit = static_cast<double>(new_width) / (2.5 * 3.141592653589793);
        double amplitude = new_height * 0.16;
        plot_cos(canvas, ox, oy, px_per_unit, amplitude, Pixel{0, 0, 255}, 2);

        std::cout << "[5/5] Сохранение в " << output_file << "..." << std::endl;
        write_ppm_p3(output_file, canvas);

        std::cout << "\nУспех! Результат записан." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
