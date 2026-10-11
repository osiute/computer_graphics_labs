#include <iostream>
#include <string>
#include "../common_utils/image.h"
#include "../common_utils/netpbm.h"
#include "filters.h"

int main() {
    std::setlocale(LC_ALL, "Russian");

    std::string input_file1_path;
    std::string filtered_file1_path;
    std::string input_file2_path;
    std::string filtered_file2_path;
    std::string blend_output_path;

    std::cout << "=== Лабораторная работа #5: Попиксельные преобразования ===" << std::endl;

    std::cout << "Введите путь к первому исходному файлу (.ppm): ";
    std::cin >> input_file1_path;
    std::cout << "Укажите, в каком файле вы хотите увидеть изменённое первое изображение (.ppm): ";
    std::cin >> filtered_file1_path;

    std::cout << "Введите путь ко второму исходному файлу (.ppm): ";
    std::cin >> input_file2_path;
    std::cout << "Укажите, в каком файле вы хотите увидеть изменённое второе изображение (.ppm): ";
    std::cin >> filtered_file2_path;

    std::cout << "Введите путь для сохранения результата (.ppm): ";
    std::cin >> blend_output_path;

    try {
        std::cout << "\n[1/5] Чтение исходных изображений с диска..." << std::endl;
        Image img1 = read_ppm_p3(input_file1_path);
        Image img2 = read_ppm_p3(input_file2_path);

        std::cout << "[2/5] Применение фильтра к первому изображению и сохранение..." << std::endl;
        apply_variant_filter(img1);
        write_ppm_p3(filtered_file1_path, img1);

        std::cout << "[3/5] Применение фильтра ко второму изображению и сохранение..." << std::endl;
        apply_variant_filter(img2);
        write_ppm_p3(filtered_file2_path, img2);

        std::cout << "[4/5] Наложение измененных изображений с прозрачностью (alpha = 0.5)..." << std::endl;
        Image blended = blend_images(img1, img2, 0.5);

        std::cout << "[5/5] Сохранение итогового результата в файл " << blend_output_path << "..." << std::endl;
        write_ppm_p3(blend_output_path, blended);

        std::cout << "\nУспех! Все этапы обработки успешно завершены." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}