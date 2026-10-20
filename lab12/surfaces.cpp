#include "surfaces.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

static double choose(int n, int k) {
    if (k < 0 || k > n) return 0;
    k = std::min(k, n - k);
    double result = 1;
    for (int j = 1; j <= k; ++j)
        result *= static_cast<double>(n - k + j) / j;
    return result;
}

double bernstein(int i, int n, double t) {
    return choose(n, i) * std::pow(t, i) * std::pow(1 - t, n - i);
}

Mesh generate_bezier_surface(const std::vector<std::vector<Point3D>>& cp, int ru, int rv) {
    if (cp.size() < 2 || cp.front().size() < 2 || ru < 1 || rv < 1)
        throw std::invalid_argument("Bezier: требуется сетка не меньше 2x2 и положительное разбиение");
    const int rows = static_cast<int>(cp.size());
    const int cols = static_cast<int>(cp.front().size());
    for (const auto& row : cp)
        if (static_cast<int>(row.size()) != cols)
            throw std::invalid_argument("Bezier: длины строк контрольной сетки не совпадают");
    Mesh result;
    result.vertices.reserve(static_cast<size_t>(ru + 1) * (rv + 1));
    for (int i = 0; i <= ru; ++i) {
        double u = static_cast<double>(i) / ru;
        std::vector<double> bu(rows);
        for (int r = 0; r < rows; ++r) bu[r] = bernstein(r, rows - 1, u);
        for (int j = 0; j <= rv; ++j) {
            double v = static_cast<double>(j) / rv;
            Point3D p{0, 0, 0};
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < cols; ++c) {
                    double w = bu[r] * bernstein(c, cols - 1, v);
                    p.x += w * cp[r][c].x;
                    p.y += w * cp[r][c].y;
                    p.z += w * cp[r][c].z;
                }
            result.vertices.push_back(p);
        }
    }
    const int stride = rv + 1;
    for (int i = 0; i < ru; ++i)
        for (int j = 0; j < rv; ++j) {
            int a = i * stride + j, b = (i + 1) * stride + j;
            result.faces.push_back({{a, b, b + 1, a + 1}});
        }
    return result;
}

// В ориентированной замкнутой сетке каждому ребру a->b соответствует b->a.
// Важны и координаты новых точек, и правильный циклический порядок граней.
static Mesh doo_sabin_step(const Mesh& mesh) {
    using Edge = std::pair<int, int>;
    struct HalfEdge { int face, corner; };
    std::map<Edge, HalfEdge> halfedges;
    std::vector<std::vector<int>> corners(mesh.faces.size());
    std::vector<std::vector<std::pair<int, int>>> incident(mesh.vertices.size());

    for (int f = 0; f < static_cast<int>(mesh.faces.size()); ++f) {
        const auto& ids = mesh.faces[f].vertex_indices;
        if (ids.size() < 3) throw std::invalid_argument("Doo-Sabin: грань имеет менее 3 вершин");
        corners[f].resize(ids.size());
        for (int i = 0; i < static_cast<int>(ids.size()); ++i) {
            int a = ids[i], b = ids[(i + 1) % ids.size()];
            if (a < 0 || b < 0 || a >= static_cast<int>(mesh.vertices.size()) || b >= static_cast<int>(mesh.vertices.size()) || a == b)
                throw std::invalid_argument("Doo-Sabin: некорректный индекс вершины или вырожденное ребро");
            if (!halfedges.emplace(Edge{a, b}, HalfEdge{f, i}).second)
                throw std::invalid_argument("Doo-Sabin: повторное ориентированное ребро; проверьте обход граней");
            incident[a].push_back({f, i});
        }
    }
    for (const auto& item : halfedges)
        if (!halfedges.count({item.first.second, item.first.first}))
            throw std::invalid_argument("Doo-Sabin: требуется замкнутая согласованно ориентированная сетка");

    Mesh next;
    for (int f = 0; f < static_cast<int>(mesh.faces.size()); ++f) {
        const auto& ids = mesh.faces[f].vertex_indices;
        const int n = static_cast<int>(ids.size());
        Point3D center{0, 0, 0};
        for (int id : ids) {
            center.x += mesh.vertices[id].x;
            center.y += mesh.vertices[id].y;
            center.z += mesh.vertices[id].z;
        }
        center.x /= n; center.y /= n; center.z /= n;
        for (int i = 0; i < n; ++i) {
            const Point3D& p = mesh.vertices[ids[i]];
            const Point3D& prev = mesh.vertices[ids[(i + n - 1) % n]];
            const Point3D& after = mesh.vertices[ids[(i + 1) % n]];
            // Среднее вершины, двух середин смежных рёбер и центра грани.
            Point3D q{0.5 * p.x + 0.125 * (prev.x + after.x) + 0.25 * center.x,
                      0.5 * p.y + 0.125 * (prev.y + after.y) + 0.25 * center.y,
                      0.5 * p.z + 0.125 * (prev.z + after.z) + 0.25 * center.z};
            corners[f][i] = static_cast<int>(next.vertices.size());
            next.vertices.push_back(q);
        }
    }

    // F-грани: по одной внутри каждой исходной грани.
    for (const auto& row : corners) next.faces.push_back({row});

    // E-грани: четыре точки около каждого старого ребра, в правильном порядке.
    for (const auto& item : halfedges) {
        int a = item.first.first, b = item.first.second;
        if (a > b) continue; // одно неориентированное ребро один раз
        const auto& h = item.second;
        const auto& reverse = halfedges.at({b, a});
        int nf = static_cast<int>(mesh.faces[h.face].vertex_indices.size());
        int ng = static_cast<int>(mesh.faces[reverse.face].vertex_indices.size());
        next.faces.push_back({{
            corners[h.face][(h.corner + 1) % nf],
            corners[h.face][h.corner],
            corners[reverse.face][(reverse.corner + 1) % ng],
            corners[reverse.face][reverse.corner]
        }});
    }

    // V-грани: обходим инцидентные грани вокруг старой вершины,
    // переходя через общее ребро, а не в порядке хранения в std::map.
    for (int v = 0; v < static_cast<int>(incident.size()); ++v) {
        if (incident[v].empty()) continue;
        const auto start = incident[v].front();
        auto current = start;
        std::vector<int> polygon;
        for (size_t step = 0; step < incident[v].size(); ++step) {
            int f = current.first, i = current.second;
            polygon.push_back(corners[f][i]);
            const auto& face = mesh.faces[f].vertex_indices;
            int next_v = face[(i + 1) % face.size()];
            auto opp = halfedges.find({next_v, v});
            if (opp == halfedges.end()) throw std::invalid_argument("Doo-Sabin: разрыв сетки");
            int g = opp->second.face;
            int j = (opp->second.corner + 1) % mesh.faces[g].vertex_indices.size();
            current = {g, j};
            if (current == start) break;
        }
        if (polygon.size() != incident[v].size() || current != start)
            throw std::invalid_argument("Doo-Sabin: нарушен циклический обход вокруг вершины");
        if (polygon.size() >= 3) {
            std::reverse(polygon.begin(), polygon.end());
            next.faces.push_back({polygon});
        }
    }
    return next;
}

Mesh subdivide_doo_sabin(const Mesh& input_mesh, int iterations) {
    if (iterations < 0) throw std::invalid_argument("Число итераций не может быть отрицательным");
    Mesh current = input_mesh;
    for (int i = 0; i < iterations; ++i) current = doo_sabin_step(current);
    return current;
}
