#pragma once
#include "image.h"
#include <vector>

// Структура точки на плоскости
struct Point {
    int x;
    int y;
};

// Структура прямоугольного окна отсечения
struct Rect {
    int xmin, ymin;
    int xmax, ymax;
};

// Структура отрезка
struct Line {
    Point p1;
    Point p2;
};

// Рисование линии алгоритмом Брезенхема
void draw_line(Image& img, int x0, int y0, int x1, int y1, const Pixel& color);

// Рисование прямоугольного окна отсечения
void draw_rect(Image& img, const Rect& rect, const Pixel& color);

// Алгоритм Коэна-Сазерленда для отсечения отрезка прямоугольным окном.
// Возвращает true, если отрезок виден (хотя бы частично), и обновляет координаты (x0, y0)-(x1, y1).
bool cohen_sutherland_clip(int& x0, int& y0, int& x1, int& y1, const Rect& rect);