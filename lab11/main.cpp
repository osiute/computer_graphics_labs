#include <iostream>
#include <string>
#include <cmath>
#include "image.h"
#include "netpbm.h"
#include "platonic.h"
#include "obj_io.h"

// Алгоритм Брезенхема для рисования линий каркаса на холсте
void draw_line(Image& img, int x0, int y0, int x1, int y1, const Pixel& color) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && x0 < img.width && y0 >= 0 && y0 < img.height) {
            img.pixels[y0 * img.width + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

int main() {
    // Настройка локали для корректного вывода кириллицы в консоли
    std::setlocale(LC_ALL, "Russian");
    
    std::cout << "=== Лабораторная работа #11: Проективные преобразования (Вариант 2) ===" << std::endl;

    // Параметры холста
    int width = 800;
    int height = 600;
    Image img;
    img.width = width;
    img.height = height;
    img.pixels.assign(width * height, Pixel{255, 255, 255}); // Заливка белым цветом

    // Выбор фигуры
    int choice = 1;
    std::cout << "Выберите фигуру для визуализации:\n"
              << " 1 - Гексаэдр (Куб)\n"
              << " 2 - Додекаэдр\n"
              << "Ваш выбор: ";
    std::cin >> choice;

    WireframeModel model;
    if (choice == 2) {
        model = create_dodecahedron();
        std::cout << "Выбран Додекаэдр (20 вершин, 30 ребер)." << std::endl;
    } else {
        model = create_cube();
        std::cout << "Выбран Гексаэдр / Куб (8 вершин, 12 ребер)." << std::endl;
    }

    // Параметр проекции (фокусное расстояние d)
    double d = 200.0;
    std::cout << "Введите фокусное расстояние d (рекомендуется 300-500): ";
    std::cin >> d;

    std::string filename;
    std::cout << "Введите имя файла для сохранения (.ppm): ";
    std::cin >> filename;

    // Проецирование 3D вершин в 2D экранные координаты
    std::vector<Point3D> projected_vertices(model.vertices.size());
    double center_x = width / 2.0;
    double center_y = height / 2.0;

    for (size_t i = 0; i < model.vertices.size(); ++i) {
        projected_vertices[i] = apply_perspective(model.vertices[i], d, center_x, center_y);
    }

    // Отрисовка рёбер каркаса (насыщенный синий цвет)
    Pixel edge_color = {20, 60, 180};
    for (const auto& edge : model.edges) {
        const auto& p1 = projected_vertices[edge.v1];
        const auto& p2 = projected_vertices[edge.v2];
        draw_line(img, static_cast<int>(p1.x), static_cast<int>(p1.y),
                       static_cast<int>(p2.x), static_cast<int>(p2.y), edge_color);
    }

    // Сохранение результата в PPM файл с помощью нашей утилиты
    write_ppm_p3(filename, img);
    std::cout << "Готово! Каркасное изображение успешно сохранено в файл: " << filename << std::endl;

    std::string obj_name = filename;
    auto dot = obj_name.find_last_of('.');
    if (dot != std::string::npos) {
        obj_name = obj_name.substr(0, dot);
    }
    obj_name += ".obj";

    std::vector<ObjVert> verts;
    verts.reserve(model.vertices.size());
    for (const auto& p : model.vertices) {
        verts.push_back({p.x, p.y, p.z});
    }
    std::vector<ObjFace> faces;
    for (const auto& f : model.faces) {
        faces.push_back({f});
    }
    std::string object_tag = (choice == 2) ? "Dodecahedron" : "Cube";
    if (write_wavefront_obj(obj_name, object_tag, verts, faces)) {
        std::cout << "OBJ сохранён: " << obj_name << std::endl;
    } else {
        std::cerr << "Не удалось записать OBJ: " << obj_name << std::endl;
    }

    return 0;
}