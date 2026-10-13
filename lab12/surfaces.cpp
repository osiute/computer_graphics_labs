#include "surfaces.h"
#include <cmath>

// Вспомогательная функция для вычисления факториала
static long long factorial(int n) {
    if (n <= 1) return 1;
    long long res = 1;
    for (int i = 2; i <= n; ++i) res *= i;
    return res;
}

// Биномиальный коэффициент C(n, i) = n! / (i! * (n - i)!)
static double binomial_coefficient(int n, int i) {
    if (i < 0 || i > n) return 0.0;
    return static_cast<double>(factorial(n)) / (factorial(i) * factorial(n - i));
}

double bernstein(int i, int n, double t) {
    return binomial_coefficient(n, i) * std::pow(t, i) * std::pow(1.0 - t, n - i);
}

Mesh generate_bezier_surface(const std::vector<std::vector<Point3D>>& control_points, int resolution_u, int resolution_v) {
    Mesh surface_mesh;
    int n = control_points.size() - 1;
    if (n < 0) return surface_mesh;
    int m = control_points[0].size() - 1;
    if (m < 0) return surface_mesh;

    int grid_w = resolution_u + 1;
    int grid_h = resolution_v + 1;

    // 1. Вычисляем точки поверхности по параметрам u и v от 0 до 1
    surface_mesh.vertices.reserve(grid_w * grid_h);

    for (int i = 0; i <= resolution_u; ++i) {
        double u = static_cast<double>(i) / resolution_u;
        for (int j = 0; j <= resolution_v; ++j) {
            double v = static_cast<double>(j) / resolution_v;

            Point3D p{0.0, 0.0, 0.0};
            for (int r = 0; r <= n; ++r) {
                double b_r = bernstein(r, n, u);
                for (int c = 0; c <= m; ++c) {
                    double b_c = bernstein(c, m, v);
                    double weight = b_r * b_c;
                    p.x += control_points[r][c].x * weight;
                    p.y += control_points[r][c].y * weight;
                    p.z += control_points[r][c].z * weight;
                }
            }
            surface_mesh.vertices.push_back(p);
        }
    }

    // 2. Формируем четырехугольные грани (сетку)
    for (int i = 0; i < resolution_u; ++i) {
        for (int j = 0; j < resolution_v; ++j) {
            int idx00 = i * grid_h + j;
            int idx10 = (i + 1) * grid_h + j;
            int idx11 = (i + 1) * grid_h + (j + 1);
            int idx01 = i * grid_h + (j + 1);

            surface_mesh.faces.push_back({ {idx00, idx10, idx11, idx01} });
        }
    }

    return surface_mesh;
}