#include <iostream>
#include <string>
#include "image.h"
#include "netpbm.h"
#include "filters.h"

int main() {
    std::setlocale(LC_ALL, "Russian");

    try {
        std::cout << "1. Читаем image1.ppm и image2.ppm...\n";
        Image img1 = read_ppm_p3("image1.ppm");
        Image img2 = read_ppm_p3("image2.ppm");

        std::cout << "2. Применяем фильтр по варианту (насыщенность +30% для V < 0.5)...\n";
        apply_variant_filter(img1);
        write_ppm_p3("result_filter.ppm", img1);
        std::cout << "Фильтр применен, результат сохранен в result_filter.ppm\n";

        std::cout << "3. Проверяем наложение (альфа-блендинг, alpha = 0.5)...\n";
        Image blended = blend_images(img1, img2, 0.5);
        write_ppm_p3("result_blend.ppm", blended);
        std::cout << "Наложение выполнено, результат сохранен в result_blend.ppm\n";

        std::cout << "\nВсё работает корректно!\n";
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }

    std::string file1_path;
    std::string file2_path;
    std::string output_path;

    std::cout << "=== Лабораторная работа #5: Попиксельные преобразования ===" << std::endl;

    std::cout << "Введите путь к первому исходному файлу (.ppm): ";
    std::cin >> file1_path;

    std::cout << "Введите путь ко второму исходному файлу (.ppm): ";
    std::cin >> file2_path;

    std::cout << "Введите путь для сохранения результата (.ppm): ";
    std::cin >> output_path;

    std::cout << "\n--- Проверка введенных данных (Заглушка) ---" << std::endl;
    std::cout << "Первое изображение: " << file1_path << std::endl;
    std::cout << "Второе изображение: " << file2_path << std::endl;
    std::cout << "Файл результата:    " << output_path << std::endl;
    std::cout << "Статус: интерактивный ввод работает успешно. Готовы подключать логику чтения и фильтров!" << std::endl;

    return 0;
}