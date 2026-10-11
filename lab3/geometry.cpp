#include "geometry.h"
#include <cmath>
#include <cassert>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::vector<Point> calculate_pentagram_vertices(int cx, int cy, int radius, double initial_angle_deg) {
    std::vector<Point> vertices(5);
    double initial_angle_rad = initial_angle_deg * M_PI / 180.0;
    double angle_step = 2.0 * M_PI / 5.0;

    for (int i = 0; i < 5; ++i) {
        double current_angle = initial_angle_rad + i * angle_step;
        
        // Вычисляем координаты с плавающей точкой
        double x = cx + radius * std::cos(current_angle);
        double y = cy - radius * std::sin(current_angle);
        
        // Округляем до ближайшего целого пикселя и сохраняем
        vertices[i] = {
            static_cast<int>(std::round(x)), 
            static_cast<int>(std::round(y))
        };
    }

    return vertices;
}

std::vector<Segment> get_pentagram_segments(const std::vector<Point>& vertices) {
    assert (vertices.size() == 5);

    std::vector<Segment> segments;
    // Порядок соединения: 0 -> 2 -> 4 -> 1 -> 3 -> 0
    int order[5] = {0, 2, 4, 1, 3};

    for (int i = 0; i < 5; ++i) {
        Point p1 = vertices[order[i]];
        Point p2 = vertices[order[(i + 1) % 5]]; // Следующая вершина в цепочке
        segments.push_back({p1, p2});
    }

    return segments;
}

std::vector<Segment> get_pentagram_segments(int cx, int cy, int radius, double initial_angle_deg) {
    std::vector<Point> vertices = calculate_pentagram_vertices(cx, cy, radius, initial_angle_deg);
    std::vector<Segment> segments = get_pentagram_segments(vertices);
    return segments;
}

void offset_segments(std::vector<Segment>& segments, int dx, int dy) {
    for (auto& segment : segments) {
        segment.p1.x += dx;
        segment.p1.y += dy;
        segment.p2.x += dx;
        segment.p2.y += dy;
    }
}