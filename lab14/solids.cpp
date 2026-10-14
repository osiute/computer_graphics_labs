#include "lab14.h"
#include <cmath>
#include <iostream>

Mesh create_parallelepiped(double hx, double hy, double hz) {
    // Пол на y=0 (плоскость сетки). hy здесь — высота, не полуразмер.
    double H = hy;
    Mesh mesh;
    mesh.vertices = {
        {-hx, 0.0, -hz}, {hx, 0.0, -hz}, {hx, H, -hz}, {-hx, H, -hz},
        {-hx, 0.0,  hz}, {hx, 0.0,  hz}, {hx, H,  hz}, {-hx, H,  hz}
    };
    mesh.faces = {
        {{0, 3, 2, 1}},
        {{4, 5, 6, 7}},
        {{0, 1, 5, 4}},
        {{3, 7, 6, 2}},
        {{0, 4, 7, 3}},
        {{1, 2, 6, 5}}
    };
    return mesh;
}

Mesh create_triangular_prism(double radius, double height) {
    // Основание на y=0, вершины на линиях сетки (шаг 20).
    (void)radius;
    Mesh mesh;
    mesh.vertices = {
        { 80.0, 0.0,  40.0},
        {-80.0, 0.0,  40.0},
        {  0.0, 0.0, -80.0},
        { 80.0, height,  40.0},
        {-80.0, height,  40.0},
        {  0.0, height, -80.0}
    };
    mesh.faces = {
        {{0, 2, 1}},
        {{3, 4, 5}},
        {{0, 1, 4, 3}},
        {{1, 2, 5, 4}},
        {{2, 0, 3, 5}}
    };
    return mesh;
}

Mesh create_house() {
    // Пол на y=0, контур кратный шагу сетки 20.
    Mesh mesh;
    mesh.vertices = {
        {-80.0,  0.0, -40.0},
        { 80.0,  0.0, -40.0},
        { 80.0,  0.0,  40.0},
        {-80.0,  0.0,  40.0},
        {-80.0, 80.0, -40.0},
        { 80.0, 80.0, -40.0},
        { 80.0, 80.0,  40.0},
        {-80.0, 80.0,  40.0},
        {  0.0, 120.0, -40.0},
        {  0.0, 120.0,  40.0}
    };
    mesh.faces = {
        {{0, 1, 2, 3}},
        {{0, 4, 8, 5, 1}},
        {{3, 2, 6, 9, 7}},
        {{0, 3, 7, 4}},
        {{1, 5, 6, 2}},
        {{4, 7, 9, 8}},
        {{5, 6, 9, 8}}
    };
    return mesh;
}

Mesh load_mesh_from_stdin() {
    Mesh mesh;
    int n = 0;
    std::cout << "Число вершин N (0 = домик-заготовка): ";
    std::cin >> n;
    if (n <= 0) {
        std::cout << "Заготовка: домик, 10 вершин, 7 граней." << std::endl;
        return create_house();
    }
    mesh.vertices.resize(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        std::cout << "Вершина " << (i + 1) << " x y z: ";
        std::cin >> mesh.vertices[i].x >> mesh.vertices[i].y >> mesh.vertices[i].z;
    }
    int m = 0;
    std::cout << "Число граней M: ";
    std::cin >> m;
    for (int f = 0; f < m; ++f) {
        int k = 0;
        std::cout << "Грань " << (f + 1) << ": k затем k индексов с 1: ";
        std::cin >> k;
        Face face;
        face.vertex_indices.resize(static_cast<size_t>(k));
        for (int t = 0; t < k; ++t) {
            int idx = 0;
            std::cin >> idx;
            face.vertex_indices[t] = idx - 1;
        }
        mesh.faces.push_back(face);
    }
    return mesh;
}
