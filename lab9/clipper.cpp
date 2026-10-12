#include "clipper.h"
#include <algorithm>
#include <cmath>

// Коды областей для алгоритма Коэна-Сазерленда
const int INSIDE = 0; // 0000 - внутри окна
const int LEFT = 1;   // 0001 - слева
const int RIGHT = 2;  // 0010 - справа
const int BOTTOM = 4; // 0100 - снизу
const int TOP = 8;    // 1000 - сверху

// Вычисление 4-битного кода точки относительно окна
static int compute_out_code(double x, double y, const Rect& rect) {
    int code = INSIDE;

    if (x < rect.xmin)      // Слева от окна
        code |= LEFT;
    else if (x > rect.xmax) // Справа от окна
        code |= RIGHT;
    
    if (y < rect.ymin)      // Снизу от окна
        code |= BOTTOM;     
    else if (y > rect.ymax) // Сверху от окна
        code |= TOP;

    return code;
}

// Алгоритм Коэна-Сазерленда для отсечения отрезка
bool cohen_sutherland_clip(int& x0, int& y0, int& x1, int& y1, const Rect& rect) {
    double dx0 = x0, dy0 = y0;
    double dx1 = x1, dy1 = y1;

    int code0 = compute_out_code(dx0, dy0, rect);
    int code1 = compute_out_code(dx1, dy1, rect);
    bool accept = false;

    while (true) {
        if (!(code0 | code1)) {
            // Оба конца внутри окна
            accept = true;
            break;
        } else if (code0 & code1) {
            // Оба конца в одной внешней зоне — отрезок полностью вне окна
            break;
        } else {
            int out_code = code0 ? code0 : code1;
            double x = 0.0, y = 0.0;

            // Находим точку пересечения с границей
            if (out_code & TOP) {
                x = dx0 + (dx1 - dx0) * (rect.ymax - dy0) / (dy1 - dy0);
                y = rect.ymax;
            } else if (out_code & BOTTOM) {
                x = dx0 + (dx1 - dx0) * (rect.ymin - dy0) / (dy1 - dy0);
                y = rect.ymin;
            } else if (out_code & RIGHT) {
                y = dy0 + (dy1 - dy0) * (rect.xmax - dx0) / (dx1 - dx0);
                x = rect.xmax;
            } else if (out_code & LEFT) {
                y = dy0 + (dy1 - dy0) * (rect.xmin - dx0) / (dx1 - dx0);
                x = rect.xmin;
            }

            // Заменяем внешнюю точку точкой пересечения и повторяем
            if (out_code == code0) {
                dx0 = x;
                dy0 = y;
                code0 = compute_out_code(dx0, dy0, rect);
            } else {
                dx1 = x;
                dy1 = y;
                code1 = compute_out_code(dx1, dy1, rect);
            }
        }
    }

    if (accept) {
        x0 = static_cast<int>(std::round(dx0));
        y0 = static_cast<int>(std::round(dy0));
        x1 = static_cast<int>(std::round(dx1));
        y1 = static_cast<int>(std::round(dy1));
        return true;
    }

    return false;
}

// Алгоритм Брезенхема для рисования линий
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

// Рисование прямоугольного окна по 4 сторонам
void draw_rect(Image& img, const Rect& rect, const Pixel& color) {
    draw_line(img, rect.xmin, rect.ymin, rect.xmax, rect.ymin, color); // Верхняя
    draw_line(img, rect.xmax, rect.ymin, rect.xmax, rect.ymax, color); // Правая
    draw_line(img, rect.xmax, rect.ymax, rect.xmin, rect.ymax, color); // Нижняя
    draw_line(img, rect.xmin, rect.ymax, rect.xmin, rect.ymin, color); // Левая
}