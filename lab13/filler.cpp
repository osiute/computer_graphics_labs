#include "filler.h"
#include <algorithm>
#include <cmath>

void rasterize_polygon_fill(Image& img, const std::vector<Point2D>& polygon, const Pixel& color) {
    if (polygon.size() < 3) return;

    int min_y = img.height;
    int max_y = 0;
    for (const auto& p : polygon) {
        int py = static_cast<int>(std::round(p.y));
        min_y = std::min(min_y, py);
        max_y = std::max(max_y, py);
    }

    min_y = std::max(0, min_y);
    max_y = std::min(img.height - 1, max_y);

    for (int y = min_y; y <= max_y; ++y) {
        std::vector<double> intersections;
        size_t n = polygon.size();

        for (size_t i = 0; i < n; ++i) {
            const Point2D& p1 = polygon[i];
            const Point2D& p2 = polygon[(i + 1) % n];

            if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y)) {
                if (p1.y != p2.y) {
                    double x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                    intersections.push_back(x);
                }
            }
        }

        std::sort(intersections.begin(), intersections.end());

        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
            int x_start = std::max(0, static_cast<int>(std::ceil(intersections[i])));
            int x_end = std::min(img.width - 1, static_cast<int>(std::floor(intersections[i + 1])));

            for (int x = x_start; x <= x_end; ++x) {
                if (x >= 0 && x < img.width && y >= 0 && y < img.height) {
                    img.pixels[y * img.width + x] = color;
                }
            }
        }
    }
}