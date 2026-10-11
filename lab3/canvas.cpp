#include "canvas.h"
#include <fstream>

Canvas create_canvas(int width, int height) {
    return Canvas(height, std::vector<int>(width, 0));
}

void save_canvas_to_pbm(const std::string& filename, const Canvas& canvas) {
    int height = canvas.size();
    if (height == 0) return;
    int width = canvas[0].size();

    std::ofstream file(filename);
    if (!file.is_open()) return;

    // Заголовок PBM формата P1 (ASCII монохром)
    file << "P1\n";
    file << width << " " << height << "\n";

    // Запись матрицы пикселей
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            file << canvas[y][x] << " ";
        }
        file << "\n";
    }

    file.close();
}

void draw_grid(Canvas& canvas, int cx, int cy) {
    int height = canvas.size();
    int width = canvas[0].size();

    // Вертикальная линия разметки
    for (int y = 0; y < height; ++y) {
        if (cx >= 0 && cx < width) canvas[y][cx] = 1;
    }

    // Горизонтальная линия разметки
    for (int x = 0; x < width; ++x) {
        if (cy >= 0 && cy < height) canvas[cy][x] = 1;
    }
}