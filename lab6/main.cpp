#include <iostream>
#include <string>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
#include "image.h"
#include "netpbm.h"
#include "lab6.h"

static void save_both(const std::string& prefix, const Image& img) {
    write_ppm_p3(prefix + ".ppm", img);
    save_png(prefix + ".png", img);
}

// Загружает картинку, делает масштаб, перенос, обратное аффинное и i=2^{x'}, сохраняет файлы
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Лабораторная работа #6: Геометрические преобразования (вариант 2) ===" << std::endl;
    std::cout << "Аффинные: масштабирование i=a*x', j=e*y' и перенос i=x'+c, j=y'+f." << std::endl;
    std::cout << "Функция: i = 2^{x'}, j = y'." << std::endl;

    std::string input_file;
    std::string prefix;
    double a = 1.0;
    double e = 1.0;
    double c = 0.0;
    double f = 0.0;

    std::cout << "Введите путь к исходному изображению (ppm/jpg/png): ";
    std::cin >> input_file;
    std::cout << "Масштаб a по X (например, 0.75): ";
    std::cin >> a;
    std::cout << "Масштаб e по Y (например, 0.75): ";
    std::cin >> e;
    std::cout << "Перенос c по X в пикселях (например, 80): ";
    std::cin >> c;
    std::cout << "Перенос f вверх по Y в пикселях (например, 80): ";
    std::cin >> f;
    std::cout << "Префикс файлов результата (например, out): ";
    std::cin >> prefix;

    if (a == 0.0 || e == 0.0) {
        std::cerr << "Масштаб a и e не должны быть нулевыми." << std::endl;
        return 1;
    }

    try {
        std::cout << "\n[1/6] Чтение исходного изображения..." << std::endl;
        Image src = load_image(input_file);

        std::cout << "[2/6] Масштабирование (обратное отображение)..." << std::endl;
        Image scaled = affine_inverse(src, a, e, 0.0, 0.0);
        save_both(prefix + "_scale", scaled);

        std::cout << "[3/6] Перенос (обратное отображение)..." << std::endl;
        Image shifted = affine_inverse(src, 1.0, 1.0, c, f);
        save_both(prefix + "_translate", shifted);

        std::cout << "[4/6] Прямое и обратное аффинные (масштаб+перенос)..." << std::endl;
        Image fwd = affine_forward(src, a, e, c, f);
        save_both(prefix + "_affine_forward", fwd);
        Image inv = affine_inverse(src, a, e, c, f);
        save_both(prefix + "_affine_inverse", inv);

        std::cout << "[5/6] Восстановление исходника обратным аффинным..." << std::endl;
        Image restored = affine_restore(inv, a, e, c, f);
        save_both(prefix + "_restored", restored);

        std::cout << "[6/6] Функциональное i = 2^{x'}, j = y'..." << std::endl;
        Image func = functional_pow2_x(src);
        save_both(prefix + "_functional", func);

        std::cout << "\nУспех. Записаны PPM и PNG:" << std::endl;
        std::cout << "  " << prefix << "_scale" << std::endl;
        std::cout << "  " << prefix << "_translate" << std::endl;
        std::cout << "  " << prefix << "_affine_forward" << std::endl;
        std::cout << "  " << prefix << "_affine_inverse" << std::endl;
        std::cout << "  " << prefix << "_restored" << std::endl;
        std::cout << "  " << prefix << "_functional" << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
