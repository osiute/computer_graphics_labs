#include "geometry.h"
#include <cmath>
#include <algorithm>

Point3D apply_perspective(const Point3D& p, double focal_distance, double center_x, double center_y) {
    // Смещение по оси Z, чтобы камера находилась на расстоянии от объекта
    double z_dist = p.z + 500.0;
    if (z_dist < 1.0) z_dist = 1.0;

    double xp = (focal_distance * p.x) / z_dist;
    double yp = (focal_distance * p.y) / z_dist;

    return { center_x + xp, center_y - yp, z_dist };
}

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

void draw_mesh(Image& img, const Mesh& mesh, double focal_distance, const Pixel& edge_color, const Pixel& vertex_color) {
    double cx = img.width / 2.0;
    double cy = img.height / 2.0;

    // Проецируем все вершины из 3D в 2D
    std::vector<Point3D> projected(mesh.vertices.size());
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        projected[i] = apply_perspective(mesh.vertices[i], focal_distance, cx, cy);
    }

    // Рисуем ребра (грани)
    for (const auto& face : mesh.faces) {
        int k = face.vertex_indices.size();
        for (int i = 0; i < k; ++i) {
            int idx1 = face.vertex_indices[i];
            int idx2 = face.vertex_indices[(i + 1) % k];
            const auto& p1 = projected[idx1];
            const auto& p2 = projected[idx2];
            draw_line(img, static_cast<int>(p1.x), static_cast<int>(p1.y),
                           static_cast<int>(p2.x), static_cast<int>(p2.y), edge_color);
        }
    }

    // Рисуем вершины (точки полигона)
    for (const auto& p : projected) {
        int px = static_cast<int>(p.x);
        int py = static_cast<int>(p.y);
        if (px >= 0 && px < img.width && py >= 0 && py < img.height) {
            img.pixels[py * img.width + px] = vertex_color;
        }
    }
}

void draw_point(Image& img, int x, int y, int radius, const Pixel& color) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= radius * radius) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < img.width && ny >= 0 && ny < img.height) {
                    img.pixels[ny * img.width + nx] = color;
                }
            }
        }
    }
}