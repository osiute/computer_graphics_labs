#pragma once
#include <vector>
#include "image.h"

struct Point3D {
    double x, y, z;
};

struct Edge {
    int v1, v2;
};

struct WireframeModel {
    std::vector<Point3D> vertices;
    std::vector<Edge> edges;
};

// Генераторы тел для Варианта 2 (Гексаэдр и Додекаэдр)
WireframeModel create_cube();
WireframeModel create_dodecahedron();

// Функция центрального проецирования по формулам из методички
Point3D apply_perspective(const Point3D& p, double d, double center_x, double center_y);