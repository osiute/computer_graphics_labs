#include <iostream>
#include <string>
#include <stdexcept>
#include "image.h"
#include "netpbm.h"
#include "filters_lab7.h"

const double SIGMA = 1.5;

int main() {
    std::setlocale(LC_ALL, "Russian");

    std::string input_file;
    std::string lpf_output_file;
    std::string hpf_output_file;
    double circle_radius = 100.0;
    double threshold = 0.7;

    std::cout << "=== Лабораторная работа #7: Пространственная фильтрация (ФНЧ и ФВЧ) ===" << std::endl;

    std::cout << "Введите путь к исходному файлу (.ppm): ";
    std::cin >> input_file;

    std::cout << "Введите путь для сохранения результата ФНЧ (.ppm): ";
    std::cin >> lpf_output_file;
    std::cout << "Введите радиус круга R для ФНЧ (например, 100): ";
    std::cin >> circle_radius;

    std::cout << "Введите путь для сохранения результата ФВЧ (.ppm): ";
    std::cin >> hpf_output_file;
    std::cout << "Введите порог яркости T для ФВЧ [0.0 - 1.0] (например, 0.7): ";
    std::cin >> threshold;

    try {
        std::cout << "\n[1/3] Чтение исходного изображения с диска..." << std::endl;
        Image img = read_ppm_p3(input_file);

        std::cout << "[2/3] Применение ФНЧ (Гаусс за пределами круга R = " << circle_radius << ")..." << std::endl;
        Image lpf_result = apply_gaussian_blur_outside_circle(img, circle_radius, SIGMA);
        write_ppm_p3(lpf_output_file, lpf_result);

        std::cout << "[3/3] Применение ФВЧ (Повышение резкости при яркости V > " << threshold << ")..." << std::endl;
        Image hpf_result = apply_sharpen_above_brightness(img, threshold);
        write_ppm_p3(hpf_output_file, hpf_result);

        std::cout << "\nУспех! Оба фильтра успешно применены, результаты записаны в разные файлы." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}