#pragma once
#include <vector>
#include "geometry.h"

// Вычисление полинома Бернштейна B_{i, n}(t)
double bernstein(int i, int n, double t);

// Генерация полигональной сетки (Mesh) для поверхности Безье
// control_points - сетка контрольных точек размера (n+1) x (m+1)
// resolution_u, resolution_v - параметры разбиения (детализация сетки)
Mesh generate_bezier_surface(const std::vector<std::vector<Point3D>>& control_points, int resolution_u, int resolution_v);