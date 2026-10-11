#include <cmath>
#include <algorithm>
#include <cassert>

#include "dda_draw.h"
#include "svg_handler.h"
#include "geometry.h"

void bresenham_draw_segments(Canvas& canvas, const std::string& filename, int x_offset, int y_offset) {
    auto segments = read_segments_from_svg(filename);
    offset_segments(segments, x_offset, y_offset);
    int height = canvas.size();
    assert (height > 0);
    int width = canvas[0].size();

    for (const auto& seg : segments) {
        int x0 = seg.p1.x;
        int y0 = seg.p1.y;
        int x1 = seg.p2.x;
        int y1 = seg.p2.y;

        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);

        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;

        // Проверяем, является ли отрезок «крутым» (наклон по вертикали больше, чем по горизонтали)
        bool steep = dy > dx;
        if (steep) {
            std::swap(dx, dy);
        }

        double error = 0.0;
        double deltaerr = (dx == 0) ? 0.0 : static_cast<double>(dy) / dx;

        int x = x0;
        int y = y0;

        for (int i = 0; i <= dx; ++i) {
            // Проверка границ холста
            if (x >= 0 && x < width && y >= 0 && y < height) {
                canvas[y][x] = 1;
            }

            error += deltaerr;
            if (error >= 0.5) {
                if (steep) {
                    x += sx;
                } else {
                    y += sy;
                }
                error -= 1.0;
            }

            if (steep) {
                y += sy;
            } else {
                x += sx;
            }
        }
    }
}