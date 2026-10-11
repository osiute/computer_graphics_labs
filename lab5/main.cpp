#include <iostream>
#include <string>
#include "image.h"
#include "netpbm.h"
#include "filters.h"

int main() {
    std::setlocale(LC_ALL, "Russian");

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