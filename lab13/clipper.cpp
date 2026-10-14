#include "clipper.h"

static bool is_inside(const Point2D& p, const Point2D& edge_start, const Point2D& edge_end) {
    return (edge_end.x - edge_start.x) * (p.y - edge_start.y) - (edge_end.y - edge_start.y) * (p.x - edge_start.x) >= 0;
}

static Point2D compute_intersection(const Point2D& cp1, const Point2D& cp2, 
                                    const Point2D& s, const Point2D& e) {
    double dcx = cp1.x - cp2.x;
    double dcy = cp1.y - cp2.y;
    double dpx = s.x - e.x;
    double dpy = s.y - e.y;

    double n1 = cp1.x * cp2.y - cp1.y * cp2.x;
    double n2 = s.x * e.y - s.y * e.x;
    double n3 = 1.0 / (dcx * dpy - dcy * dpx);

    Point2D intersection;
    intersection.x = (n1 * dpx - n2 * dcx) * n3;
    intersection.y = (n1 * dpy - n2 * dcy) * n3;
    return intersection;
}

std::vector<Point2D> sutherland_hodgman_clip(const std::vector<Point2D>& subject_polygon, 
                                             const std::vector<Point2D>& clip_window) {
    std::vector<Point2D> output_list = subject_polygon;

    if (clip_window.empty() || subject_polygon.empty()) return output_list;

    Point2D cp1 = clip_window.back();
    for (const auto& cp2 : clip_window) {
        if (output_list.empty()) break;

        std::vector<Point2D> input_list = output_list;
        output_list.clear();

        Point2D s = input_list.back();
        for (const auto& e : input_list) {
            if (is_inside(e, cp1, cp2)) {
                if (!is_inside(s, cp1, cp2)) {
                    output_list.push_back(compute_intersection(cp1, cp2, s, e));
                }
                output_list.push_back(e);
            } else if (is_inside(s, cp1, cp2)) {
                output_list.push_back(compute_intersection(cp1, cp2, s, e));
            }
            s = e;
        }
        cp1 = cp2;
    }

    return output_list;
}