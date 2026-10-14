#include <iostream>
#include <string>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
#include "image.h"
#include "netpbm.h"
#include "lab8.h"

static const char* method_name(InterpMethod m) {
    if (m == INTERP_BILINEAR) {
        return "bilinear";
    }
    if (m == INTERP_BICUBIC) {
        return "bicubic";
    }
    return "nearest";
}

static void save_both(const std::string& path_no_ext, const Image& img) {
    save_png(path_no_ext + ".png", img);
    if (static_cast<long>(img.width) * img.height <= 800L * 800L) {
        write_ppm_p3(path_no_ext + ".ppm", img);
    }
}

static void run_set(const Image& src, const std::string& prefix,
                    double sx, double sy, double angle_deg, double shear) {
    InterpMethod methods[3] = {INTERP_NEAREST, INTERP_BILINEAR, INTERP_BICUBIC};
    for (int t = 0; t < 3; ++t) {
        InterpMethod m = methods[t];
        const char* tag = method_name(m);
        std::cout << "  " << tag << ": масштаб, поворот, скос..." << std::endl;
        save_both(prefix + "_scale_" + tag, affine_scale(src, sx, sy, m));
        save_both(prefix + "_rotate_" + tag, affine_rotate(src, angle_deg, m));
        save_both(prefix + "_shear_" + tag, affine_shear_x(src, shear, m));
    }
}

// Грузит картинку, делает масштаб/поворот/скос тремя интерполяциями, пишет файлы
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Лабораторная работа #8: Интерполяция изображений ===" << std::endl;
    std::cout << "Аффинные: масштабирование, поворот, скос." << std::endl;
    std::cout << "Интерполяция: nearest, bilinear, bicubic." << std::endl;

    std::string input_file;
    std::string prefix;
    double sx1 = 1.0, sy1 = 1.0, ang1 = 0.0, sh1 = 0.0;
    double sx2 = 1.0, sy2 = 1.0, ang2 = 0.0, sh2 = 0.0;

    std::cout << "Введите путь к исходному изображению (ppm/jpg/png): ";
    std::cin >> input_file;

    std::cout << "Набор 1. Масштаб sx sy (например, 2 2): ";
    std::cin >> sx1 >> sy1;
    std::cout << "Набор 1. Поворот в градусах (например, 25): ";
    std::cin >> ang1;
    std::cout << "Набор 1. Скос по X (например, 0.3): ";
    std::cin >> sh1;

    std::cout << "Набор 2. Масштаб sx sy (например, 0.6 0.6): ";
    std::cin >> sx2 >> sy2;
    std::cout << "Набор 2. Поворот в градусах (например, 45): ";
    std::cin >> ang2;
    std::cout << "Набор 2. Скос по X (например, -0.25): ";
    std::cin >> sh2;

    std::cout << "Префикс файлов (например, out): ";
    std::cin >> prefix;

    if (sx1 == 0.0 || sy1 == 0.0 || sx2 == 0.0 || sy2 == 0.0) {
        std::cerr << "Масштаб не должен быть нулевым." << std::endl;
        return 1;
    }

    try {
        std::cout << "\nЧтение исходного изображения..." << std::endl;
        Image src = load_image(input_file);
        std::cout << "Размер: " << src.width << " x " << src.height << std::endl;

        std::cout << "\nНабор 1 (sx=" << sx1 << ", sy=" << sy1
                  << ", rot=" << ang1 << ", shear=" << sh1 << ")" << std::endl;
        run_set(src, prefix + "_p1", sx1, sy1, ang1, sh1);

        std::cout << "\nНабор 2 (sx=" << sx2 << ", sy=" << sy2
                  << ", rot=" << ang2 << ", shear=" << sh2 << ")" << std::endl;
        run_set(src, prefix + "_p2", sx2, sy2, ang2, sh2);

        std::cout << "\nУспех. Результаты в текущей папке, префикс " << prefix << std::endl;
        std::cout << "\nВыводы:" << std::endl;
        std::cout << "- nearest: ступеньки (пикселизация), края ломаные, считается быстро." << std::endl;
        std::cout << "- bilinear: переходы мягче, картинка чуть мыльная, ступенек меньше." << std::endl;
        std::cout << "- bicubic: 16 соседей, детали чётче bilinear, возможны ореолы на контрасте." << std::endl;
        std::cout << "- при увеличении разница методов видна сильнее, чем при уменьшении." << std::endl;
        std::cout << "- поворот и скос без интерполяции дают дырки/зубцы; обратное отображение их убирает." << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
