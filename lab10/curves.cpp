#include "curves.h"
#include <cmath>

// Квадратичная кривая Безье (n = 2, 3 контрольные точки)
std::vector<Point2D> generate_quadratic_bezier(const std::vector<Point2D>& control_points, int steps) {
    std::vector<Point2D> curve;
    if (control_points.size() < 3) return curve;

    const Point2D& p0 = control_points[0];
    const Point2D& p1 = control_points[1];
    const Point2D& p2 = control_points[2];

    for (int i = 0; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        double u = 1.0 - t;
        
        // Формула: B(t) = (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2
        double x = u * u * p0.x + 2.0 * u * t * p1.x + t * t * p2.x;
        double y = u * u * p0.y + 2.0 * u * t * p1.y + t * t * p2.y;
        curve.push_back({x, y});
    }
    return curve;
}

// Кубическая кривая Безье (n = 3, 4 контрольные точки)
std::vector<Point2D> generate_cubic_bezier(const std::vector<Point2D>& control_points, int steps) {
    std::vector<Point2D> curve;
    if (control_points.size() < 4) return curve;

    const Point2D& p0 = control_points[0];
    const Point2D& p1 = control_points[1];
    const Point2D& p2 = control_points[2];
    const Point2D& p3 = control_points[3];

    for (int i = 0; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        double u = 1.0 - t;
        
        // Формула: B(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)*t^2*P2 + t^3*P3
        double x = u * u * u * p0.x + 3.0 * u * u * t * p1.x + 3.0 * u * t * t * p2.x + t * t * t * p3.x;
        double y = u * u * u * p0.y + 3.0 * u * u * t * p1.y + 3.0 * u * t * t * p2.y + t * t * t * p3.y;
        curve.push_back({x, y});
    }
    return curve;
}

// Метод Чайкина (угловое отсечение)
std::vector<Point2D> generate_chaikin(const std::vector<Point2D>& control_points, int iterations) {
    if (control_points.size() < 3) return control_points;
    std::vector<Point2D> current_points = control_points;

    for (int it = 0; it < iterations; ++it) {
        std::vector<Point2D> next_points;
        int n = current_points.size();
        
        // Замкнутый или разомкнутый контур (сделаем разомкнутым с сохранением концов)
        next_points.push_back(current_points[0]);

        for (size_t i = 0; i < current_points.size() - 1; ++i) {
            const Point2D& p_i = current_points[i];
            const Point2D& p_next = current_points[i + 1];

            // Q_i = 3/4 * P_i + 1/4 * P_{i+1}
            Point2D q = { 0.75 * p_i.x + 0.25 * p_next.x, 0.75 * p_i.y + 0.25 * p_next.y };
            // R_i = 1/4 * P_i + 3/4 * P_{i+1}
            Point2D r = { 0.25 * p_i.x + 0.75 * p_next.x, 0.25 * p_i.y + 0.75 * p_next.y };

            next_points.push_back(q);
            next_points.push_back(r);
        }

        next_points.push_back(current_points.back());
        current_points = next_points;
    }

    return current_points;
}