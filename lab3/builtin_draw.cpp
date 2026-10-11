#include <cassert>

#include "builtin_draw.h"
// Отключаем лишние зависимости CImg (например, отображение через X11/Windows, 
// так как нам нужно только рисовать в буфер и сохранять в файл)
#define cimg_display 0
#include "../libraries/CImg.h"
#include "svg_handler.h"
#include "geometry.h"

using namespace cimg_library;

void builtin_draw_segments(Canvas& canvas, const std::string& filename, int x_offset, int y_offset) {
    auto segments = read_segments_from_svg(filename);
    offset_segments(segments, x_offset, y_offset);
    int height = canvas.size();
    assert (height > 0);
    int width = canvas[0].size();

    // Создаем изображение CImg размером width x height, 1 канал (монохром)
    // Инициализируем белым цветом (255)
    CImg<unsigned char> img(width, height, 1, 1, 255);

    // Цвет линии — черный (0)
    const unsigned char black_color[] = { 0 };

    // Рисуем каждый отрезок встроенным методом CImg
    for (const auto& seg : segments) {
        img.draw_line(seg.p1.x, seg.p1.y, seg.p2.x, seg.p2.y, black_color);
    }

    // Переносим данные из буфера CImg в наш общий холст Canvas
    // В CImg: 0 — черный (линия), 255 — белый (фон)
    // В нашем Canvas: 1 — линия, 0 — фон
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Если пиксель в CImg темный (например, черный или селективный от антиалиасинга), 
            // отмечаем его как 1 на нашем холсте
            if (img(x, y) < 128) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    canvas[y][x] = 1;
                }
            }
        }
    }
}