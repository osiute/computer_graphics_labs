#include "surfaces.h"
#include <cmath>
#include <map>
#include <set>
#include <vector>

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


static Mesh single_doo_sabin_iteration(const Mesh& mesh) {
    Mesh next_mesh;
    
    // Для каждой грани исходного меша вычисляем новые точки (по одной на каждую вершину грани)
    // Структура хранения новых вершин: маппинг пары (face_index, vertex_local_index) -> новый индекс в next_mesh
    std::map<std::pair<int, int>, int> face_vertex_map;

    int new_v_count = 0;

    for (size_t f_idx = 0; f_idx < mesh.faces.size(); ++f_idx) {
        const auto& face = mesh.faces[f_idx];
        int k = face.vertex_indices.size();

        // 1. Находим центр масс (центроид) текущей грани
        Point3D centroid{0, 0, 0};
        for (int idx : face.vertex_indices) {
            centroid.x += mesh.vertices[idx].x;
            centroid.y += mesh.vertices[idx].y;
            centroid.z += mesh.vertices[idx].z;
        }
        centroid.x /= k;
        centroid.y /= k;
        centroid.z /= k;

        // 2. Для каждой вершины грани создаем новую сглаженную точку по правилу Ду-Сабина
        for (int i = 0; i < k; ++i) {
            int curr_v_idx = face.vertex_indices[i];
            const auto& v = mesh.vertices[curr_v_idx];

            // Соседние вершины на этой же грани
            int prev_v_idx = face.vertex_indices[(i - 1 + k) % k];
            int next_v_idx = face.vertex_indices[(i + 1) % k];
            const auto& v_prev = mesh.vertices[prev_v_idx];
            const auto& v_next = mesh.vertices[next_v_idx];

            Point3D new_pt;
            if (k == 4) {
                // Точные коэффициенты для четырехугольных граней (куб)
                const auto& v_opp = mesh.vertices[face.vertex_indices[(i + 2) % k]];
                new_pt.x = (9.0 * v.x + 3.0 * v_prev.x + 3.0 * v_next.x + 1.0 * v_opp.x) / 16.0;
                new_pt.y = (9.0 * v.y + 3.0 * v_prev.y + 3.0 * v_next.y + 1.0 * v_opp.y) / 16.0;
                new_pt.z = (9.0 * v.z + 3.0 * v_prev.z + 3.0 * v_next.z + 1.0 * v_opp.z) / 16.0;
            } else {
                // Общий случай для n-угольников
                new_pt.x = 0.25 * v.x + 0.5 * centroid.x + 0.125 * (v_prev.x + v_next.x);
                new_pt.y = 0.25 * v.y + 0.5 * centroid.y + 0.125 * (v_prev.y + v_next.y);
                new_pt.z = 0.25 * v.z + 0.5 * centroid.z + 0.125 * (v_prev.z + v_next.z);
            }

            face_vertex_map[{f_idx, i}] = new_v_count++;
            next_mesh.vertices.push_back(new_pt);
        }
    }

    // 3. Формируем новые типы граней (F-грани, E-грани, V-грани):
    // А) Новые грани внутри старых граней (F-faces)
    for (size_t f_idx = 0; f_idx < mesh.faces.size(); ++f_idx) {
        int k = mesh.faces[f_idx].vertex_indices.size();
        Face new_face;
        for (int i = 0; i < k; ++i) {
            new_face.vertex_indices.push_back(face_vertex_map[{f_idx, i}]);
        }
        next_mesh.faces.push_back(new_face);
    }

    // Б) Грани вокруг ребер (E-faces) и вершин (V-faces)
    // Собираем информацию о том, в каких гранях участвует каждая вершина исходного меша
    std::map<int, std::vector<std::pair<int, int>>> vertex_to_face_corners;
    for (size_t f_idx = 0; f_idx < mesh.faces.size(); ++f_idx) {
        const auto& face = mesh.faces[f_idx];
        for (int i = 0; i < face.vertex_indices.size(); ++i) {
            int v_idx = face.vertex_indices[i];
            vertex_to_face_corners[v_idx].push_back({f_idx, i});
        }
    }

    // Для каждой исходной вершины создаем V-грань (вокруг вершины)
    for (const auto& pair : vertex_to_face_corners) {
        const auto& corners = pair.second;
        Face v_face;
        for (const auto& corner : corners) {
            v_face.vertex_indices.push_back(face_vertex_map[corner]);
        }
        next_mesh.faces.push_back(v_face);
    }

    // Для каждого исходного ребра создаем E-грань (между соседними гранями, делящими ребро)
    std::set<std::pair<int, int>> processed_edges;
    for (size_t f_idx = 0; f_idx < mesh.faces.size(); ++f_idx) {
        const auto& face = mesh.faces[f_idx];
        int k = face.vertex_indices.size();
        for (int i = 0; i < k; ++i) {
            int v1 = face.vertex_indices[i];
            int v2 = face.vertex_indices[(i + 1) % k];
            
            std::pair<int, int> edge = {std::min(v1, v2), std::max(v1, v2)};
            if (processed_edges.count(edge)) continue;
            processed_edges.insert(edge);

            // Ищем две грани, содержащие это ребро
            std::vector<std::pair<int, int>> sharing_corners;
            for (const auto& corner : vertex_to_face_corners[v1]) {
                int fi = corner.first;
                int vi = corner.second;
                int nxt = mesh.faces[fi].vertex_indices[(vi + 1) % mesh.faces[fi].vertex_indices.size()];
                int prv = mesh.faces[fi].vertex_indices[(vi - 1 + mesh.faces[fi].vertex_indices.size()) % mesh.faces[fi].vertex_indices.size()];
                if (nxt == v2 || prv == v2) {
                    sharing_corners.push_back(corner);
                }
            }

            if (sharing_corners.size() == 2) {
                // Соединяем 4 точки в E-грань
                // Находим правильный порядок обхода
                int f1 = sharing_corners[0].first;
                int idx1 = sharing_corners[0].second;
                int f2 = sharing_corners[1].first;
                int idx2 = sharing_corners[1].second;

                Face e_face;
                e_face.vertex_indices = {
                    face_vertex_map[{f1, idx1}],
                    face_vertex_map[{f1, (idx1 + 1) % mesh.faces[f1].vertex_indices.size()}],
                    face_vertex_map[{f2, idx2}],
                    face_vertex_map[{f2, (idx2 + 1) % mesh.faces[f2].vertex_indices.size()}]
                };
                // Чтобы грани не пересекались, упорядочим корректно
                next_mesh.faces.push_back(e_face);
            }
        }
    }

    return next_mesh;
}

Mesh subdivide_doo_sabin(const Mesh& input_mesh, int iterations) {
    Mesh current = input_mesh;
    for (int i = 0; i < iterations; ++i) {
        current = single_doo_sabin_iteration(current);
    }
    return current;
}