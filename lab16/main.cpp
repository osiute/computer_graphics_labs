#include "obj_mtl.h"
#include "png_io.h"
#include "renderer.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#endif

namespace {
namespace fs = std::filesystem;

std::string trim(std::string text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

fs::path user_path(std::string text) {
    text = trim(std::move(text));
    if (text.size() >= 2 && ((text.front() == '"' && text.back() == '"')
                           || (text.front() == '\'' && text.back() == '\''))) {
        text = text.substr(1, text.size() - 2);
    }
    return fs::u8path(text);
}

std::string prompt(const std::string& label, const std::string& fallback) {
    std::cout << label << " [" << fallback << "]: " << std::flush;
    std::string text;
    if (!std::getline(std::cin, text)) throw std::runtime_error("Ввод завершён до получения всех параметров.");
    text = trim(std::move(text));
    return text.empty() ? fallback : text;
}

template<class T>
T number(const std::string& text, const std::string& name) {
    std::istringstream in(text);
    in.imbue(std::locale::classic());
    T value{};
    if (!(in >> value) || !(in >> std::ws).eof() || !std::isfinite(static_cast<double>(value))) {
        throw std::runtime_error("Некорректное число (" + name + "): " + text + ". Дробная часть отделяется точкой.");
    }
    return value;
}

void help(const char* program) {
    std::cout << "Лабораторная №16: OBJ + MTL, закрашивание с Z-буфером.\n\n"
              << "Интерактивный режим: " << program << "\n"
              << "С параметрами: " << program << " model.obj [output.png [azimuth elevation [width height]]]\n"
              << "Пример: " << program << " assets/house.obj build/house.png 35 25 1000 800\n"
              << "Азимут 0 — вид со стороны +Z, 90 — со стороны +X. Угол места: [-90, 90].\n"
              << "Размер: 64–4096 по каждой стороне. По умолчанию: 35 25 1000 800.\n"
              << "Пути с пробелами заключайте в кавычки. MTL ищется относительно OBJ.\n";
}

} // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    try {
#ifdef _WIN32
        // Console code page does not change narrow argv: obtain Unicode paths
        // directly from Windows and keep the rest of the interface in UTF-8.
        int wide_count = 0;
        auto* wide_args = CommandLineToArgvW(GetCommandLineW(), &wide_count);
        if (!wide_args) throw std::runtime_error("Не удалось прочитать аргументы командной строки.");
        std::unique_ptr<void, decltype(&LocalFree)> wide_owner(wide_args, &LocalFree);
        std::vector<std::string> utf8_args;
        utf8_args.reserve(wide_count);
        for (int i = 0; i < wide_count; ++i) {
            const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide_args[i], -1,
                                                 nullptr, 0, nullptr, nullptr);
            if (size <= 0) throw std::runtime_error("Некорректный Unicode в аргументах.");
            std::string value(static_cast<std::size_t>(size), '\0');
            if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide_args[i], -1,
                                     value.data(), size, nullptr, nullptr)) {
                throw std::runtime_error("Не удалось преобразовать аргументы в UTF-8.");
            }
            value.pop_back();
            utf8_args.push_back(std::move(value));
        }
        std::vector<char*> utf8_pointers;
        for (auto& value : utf8_args) utf8_pointers.push_back(value.data());
        argc = wide_count;
        argv = utf8_pointers.data();
#endif
        if (argc == 2 && std::string(argv[1]) == "--help") {
            help(argv[0]);
            return 0;
        }
        if (argc != 1 && argc != 2 && argc != 3 && argc != 5 && argc != 7) {
            help(argv[0]);
            throw std::runtime_error("Неверное количество параметров.");
        }
        std::cout << "=== Лабораторная №16: материалы OBJ/MTL ===\n";
        const bool interactive = argc == 1;
        const fs::path source = fs::u8path(LAB16_SOURCE_DIR);
        fs::path model_path = interactive
            ? user_path(prompt("Путь к OBJ", (source / "assets/house.obj").u8string()))
            : user_path(argv[1]);
        const lab16::Model model = lab16::load_obj(model_path);
        for (const auto& warning : model.warnings) std::cerr << "Предупреждение: " << warning << '\n';
        std::cout << "Загружено: " << model.mesh.vertices.size() << " вершин, "
                  << model.mesh.faces.size() << " граней, " << model.materials.size() << " материалов.\n";
        for (const auto& [name, material] : model.materials) {
            std::cout << "  " << name << ": RGB(" << static_cast<int>(material.diffuse.r)
                      << ", " << static_cast<int>(material.diffuse.g) << ", "
                      << static_cast<int>(material.diffuse.b) << ")\n";
        }
        fs::path output = interactive ? user_path(prompt("Выходной PNG", "render.png"))
                                     : argc >= 3 ? user_path(argv[2]) : fs::path("render.png");
        std::string extension = output.extension().u8string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (extension != ".png") throw std::runtime_error("Выходной файл должен иметь расширение .png.");
        // Не допускаем случайной перезаписи исходной модели через ссылку на файл.
        if (fs::exists(output) && fs::equivalent(output, model_path)) {
            throw std::runtime_error("Выходной файл совпадает с исходной моделью.");
        }
        lab16::RenderOptions options;
        if (interactive) {
            options.azimuth = number<double>(prompt("Азимут, градусы", "35"), "азимут");
            options.elevation = number<double>(prompt("Угол места, градусы", "25"), "угол места");
            options.width = number<int>(prompt("Ширина PNG", "1000"), "ширина");
            options.height = number<int>(prompt("Высота PNG", "800"), "высота");
        } else {
            if (argc >= 5) {
                options.azimuth = number<double>(argv[3], "азимут");
                options.elevation = number<double>(argv[4], "угол места");
            }
            if (argc == 7) {
                options.width = number<int>(argv[5], "ширина");
                options.height = number<int>(argv[6], "высота");
            }
        }
        const auto result = lab16::render_model(model, options);
        if (result.visible_pixels == 0) throw std::runtime_error("В выбранном ракурсе все грани видны с ребра. Измените ракурс.");
        lab16::save_png(output, result.image);
        std::cout << "Ракурс: азимут " << options.azimuth << "°, угол места " << options.elevation << "°.\n"
                  << "Отрисовано " << result.triangles << " треугольников, " << result.visible_pixels
                  << " видимых пикселей.\nPNG сохранён: " << fs::absolute(output).u8string()
                  << " (" << result.image.width << " × " << result.image.height << ")\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
