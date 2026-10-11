#include <cmath>
#include <algorithm>
#include <cassert>

#include "dda_draw.h"
#include "svg_handler.h"
#include "geometry.h"

void dda_draw_segments(Canvas& canvas, const std::string& filename, int x_offset, int y_offset) {
    auto segments = read_segments_from_svg(filename);
    offset_segments(segments, x_offset, y_offset);
    int height = canvas.size();
    assert (height > 0);
    int width = canvas[0].size();

    for (const auto& seg : segments) {
        double x0 = seg.p1.x;
        double y0 = seg.p1.y;
        double x1 = seg.p2.x;
        double y1 = seg.p2.y;

        double dx = x1 - x0;
        double dy = y1 - y0;

        // Количество шагов — это максимальная разница по осям X или Y
        int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));

        if (steps == 0) {
            // Если отрезок превратился в одну точку
            int px = static_cast<int>(std::round(x0));
            int py = static_cast<int>(std::round(y0));
            if (px >= 0 && px < width && py >= 0 && py < height) {
                canvas[py][px] = 1;
            }
            continue;
        }

        // Вычисляем приращение для каждого шага
        double x_inc = dx / steps;
        double y_inc = dy / steps;

        double x = x0;
        double y = y0;

        // Идем по шагам и закрашиваем пиксели
        for (int i = 0; i <= steps; ++i) {
            int px = static_cast<int>(std::round(x));
            int py = static_cast<int>(std::round(y));

            // Проверяем границы холста перед закрашиванием
            if (px >= 0 && px < width && py >= 0 && py < height) {
                canvas[py][px] = 1;
            }

            x += x_inc;
            y += y_inc;
        }
    }
}