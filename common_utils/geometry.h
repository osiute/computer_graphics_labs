#pragma once
#include <vector>
#include "image.h"

struct Point2D {
    double x, y;
};

struct Point3D {
    double x, y, z;
};

// Грань полигональной сетки (список индексов вершин)
struct Face {
    std::vector<int> vertex_indices;
};

// Полигональная сетка (меш) для представления 3D-объектов
struct Mesh {
    std::vector<Point3D> vertices;
    std::vector<Face> faces;
};

// Перспективное проецирование 3D-точки в 2D-координаты экрана
Point3D apply_perspective(const Point3D& p, double focal_distance, double center_x, double center_y);

// Алгоритм Брезенхема для рисования линий на растровом изображении
void draw_line(Image& img, int x0, int y0, int x1, int y1, const Pixel& color);

// Функция отрисовки всей полигональной сетки на изображении
void draw_mesh(Image& img, const Mesh& mesh, double focal_distance, const Pixel& edge_color, const Pixel& vertex_color);