#include "curves.h"

static Point2D lerp(const Point2D& a, const Point2D& b, double t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

Point2D cubic_bezier_point(const Point2D& p0, const Point2D& p1,
                           const Point2D& p2, const Point2D& p3, double t) {
    double u = 1.0 - t;
    double x = u * u * u * p0.x + 3.0 * u * u * t * p1.x + 3.0 * u * t * t * p2.x + t * t * t * p3.x;
    double y = u * u * u * p0.y + 3.0 * u * u * t * p1.y + 3.0 * u * t * t * p2.y + t * t * t * p3.y;
    return {x, y};
}

std::vector<Point2D> generate_cubic_bezier(const std::vector<Point2D>& control_points, int steps) {
    std::vector<Point2D> curve;
    if (control_points.size() < 4 || steps < 1) {
        return curve;
    }
    for (size_t s = 0; s + 3 < control_points.size(); s += 3) {
        for (int i = 0; i <= steps; ++i) {
            double t = static_cast<double>(i) / steps;
            curve.push_back(cubic_bezier_point(
                control_points[s], control_points[s + 1],
                control_points[s + 2], control_points[s + 3], t));
        }
    }
    return curve;
}

std::vector<Point2D> generate_chaikin(const std::vector<Point2D>& control_points, int iterations) {
    if (control_points.size() < 2) {
        return control_points;
    }
    std::vector<Point2D> current = control_points;
    for (int it = 0; it < iterations; ++it) {
        std::vector<Point2D> next;
        next.push_back(current.front());
        for (size_t i = 0; i + 1 < current.size(); ++i) {
            const Point2D& a = current[i];
            const Point2D& b = current[i + 1];
            next.push_back({0.75 * a.x + 0.25 * b.x, 0.75 * a.y + 0.25 * b.y});
            next.push_back({0.25 * a.x + 0.75 * b.x, 0.25 * a.y + 0.75 * b.y});
        }
        next.push_back(current.back());
        current.swap(next);
    }
    return current;
}

std::vector<std::vector<Point2D>> decasteljau_layers(const std::vector<Point2D>& points, double t) {
    std::vector<std::vector<Point2D>> layers;
    layers.push_back(points);
    std::vector<Point2D> cur = points;
    while (cur.size() > 1) {
        std::vector<Point2D> nxt;
        for (size_t i = 0; i + 1 < cur.size(); ++i) {
            nxt.push_back(lerp(cur[i], cur[i + 1], t));
        }
        layers.push_back(nxt);
        cur.swap(nxt);
    }
    return layers;
}
