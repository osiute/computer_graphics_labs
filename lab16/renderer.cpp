#include "renderer.h"
#include "triangulation.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace lab16 {
namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double radius = 100.0;
// common_utils::apply_perspective прибавляет именно 500 к Z.
constexpr double camera_distance = 500.0;

double edge(const Point3D& a, const Point3D& b, double x, double y) {
    return (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);
}

std::vector<Point3D> project_vertices(const Mesh& mesh, const RenderOptions& options) {
    Point3D low = mesh.vertices.front();
    Point3D high = low;
    for (const auto& p : mesh.vertices) {
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
            throw std::runtime_error("Координаты модели должны быть конечными числами.");
        }
        low = {std::min(low.x, p.x), std::min(low.y, p.y), std::min(low.z, p.z)};
        high = {std::max(high.x, p.x), std::max(high.y, p.y), std::max(high.z, p.z)};
    }
    const Point3D center{low.x / 2 + high.x / 2, low.y / 2 + high.y / 2,
                         low.z / 2 + high.z / 2};
    // Сначала делим на максимальную компоненту, чтобы не переполнить длину.
    double extent = 0;
    for (const auto& p : mesh.vertices) {
        extent = std::max({extent, std::abs(p.x - center.x), std::abs(p.y - center.y),
                           std::abs(p.z - center.z)});
    }
    if (!(extent > 0) || !std::isfinite(extent)) {
        throw std::runtime_error("Нулевой или слишком большой размер модели.");
    }
    double unit_radius = 0;
    std::vector<Point3D> normalized;
    normalized.reserve(mesh.vertices.size());
    for (const auto& p : mesh.vertices) {
        Point3D q{(p.x - center.x) / extent, (p.y - center.y) / extent,
                  (p.z - center.z) / extent};
        unit_radius = std::max(unit_radius, std::hypot(q.x, q.y, q.z));
        normalized.push_back(q);
    }

    const double az = std::remainder(options.azimuth, 360.0) * pi / 180.0;
    const double el = options.elevation * pi / 180.0;
    const double ca = std::cos(az), sa = std::sin(az);
    const double ce = std::cos(el), se = std::sin(el);
    // Сфера с моделью целиком попадает в кадр при любом ракурсе, глубина >= 400.
    const double focal = 0.43 * std::min(options.width, options.height)
                         * std::sqrt(camera_distance * camera_distance - radius * radius) / radius;
    std::vector<Point3D> projected;
    projected.reserve(normalized.size());
    for (auto p : normalized) {
        p.x *= radius / unit_radius;
        p.y *= radius / unit_radius;
        p.z *= radius / unit_radius;
        const Point3D view{ca * p.x - sa * p.z,
                           -se * sa * p.x + ce * p.y - se * ca * p.z,
                           -ce * sa * p.x - se * p.y - ce * ca * p.z};
        projected.push_back(apply_perspective(view, focal, options.width / 2.0,
                                              options.height / 2.0));
    }
    return projected;
}

void rasterize(const Point3D& a, const Point3D& b, const Point3D& c, Pixel color,
               Image& image, std::vector<double>& inverse_depth) {
    const double area = edge(a, b, c.x, c.y);
    if (std::abs(area) < 1e-12) return; // Грань, видимая с ребра.

    const int min_x = std::max(0, static_cast<int>(std::ceil(std::min({a.x, b.x, c.x}) - 0.5)));
    const int max_x = std::min(image.width - 1,
                             static_cast<int>(std::floor(std::max({a.x, b.x, c.x}) - 0.5)));
    const int min_y = std::max(0, static_cast<int>(std::ceil(std::min({a.y, b.y, c.y}) - 0.5)));
    const int max_y = std::min(image.height - 1,
                             static_cast<int>(std::floor(std::max({a.y, b.y, c.y}) - 0.5)));

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            const double w0 = edge(b, c, x + 0.5, y + 0.5) / area;
            const double w1 = edge(c, a, x + 0.5, y + 0.5) / area;
            const double w2 = edge(a, b, x + 0.5, y + 0.5) / area;
            if (w0 < -1e-12 || w1 < -1e-12 || w2 < -1e-12) continue;
            // В перспективе линейна обратная глубина, а не Z камеры.
            const double depth = w0 / a.z + w1 / b.z + w2 / c.z;
            const auto index = static_cast<std::size_t>(y) * image.width + x;
            if (depth > inverse_depth[index]) {
                inverse_depth[index] = depth;
                image.pixels[index] = color;
            }
        }
    }
}

} // namespace

RenderResult render_model(const Model& model, const RenderOptions& options) {
    if (options.width < 64 || options.width > 4096 || options.height < 64 || options.height > 4096) {
        throw std::runtime_error("Размер PNG должен быть от 64 до 4096 пикселей по каждой стороне.");
    }
    if (!std::isfinite(options.azimuth) || !std::isfinite(options.elevation)
        || options.elevation < -90 || options.elevation > 90) {
        throw std::runtime_error("Азимут должен быть конечным числом, угол места — от -90 до 90 градусов.");
    }
    if (model.mesh.vertices.empty() || model.mesh.faces.empty()) {
        throw std::runtime_error("В модели нет вершин или граней для отображения.");
    }
    RenderResult result;
    result.image.width = options.width;
    result.image.height = options.height;
    result.image.pixels.assign(static_cast<std::size_t>(options.width) * options.height, options.background);
    std::vector<double> inverse_depth(result.image.pixels.size(), 0.0);
    const auto projected = project_vertices(model.mesh, options);
    for (std::size_t i = 0; i < model.mesh.faces.size(); ++i) {
        std::vector<Triangle> triangles;
        try {
            triangles = triangulate_face(model.mesh, model.mesh.faces[i]);
        } catch (const std::exception& error) {
            throw std::runtime_error("Грань " + std::to_string(i + 1) + ": " + error.what());
        }
        result.triangles += triangles.size();
        const Pixel color = face_color(model, i);
        for (const auto& t : triangles) {
            rasterize(projected[t[0]], projected[t[1]], projected[t[2]], color,
                      result.image, inverse_depth);
        }
    }
    result.visible_pixels = static_cast<std::size_t>(std::count_if(
        inverse_depth.begin(), inverse_depth.end(), [](double z) { return z > 0; }));
    return result;
}

} // namespace lab16
