#pragma once
#include "image.h"
#include <string>

// Создаёт пустое изображение заданного размера и заливает его цветом fill
Image create_blank(int width, int height, Pixel fill);

// Читает картинку с диска (PPM P3, JPG, PNG) в структуру Image
Image load_image(const std::string& filename);

// Ставит пиксель color в точку (x, y), если она внутри изображения
void set_pixel(Image& img, int x, int y, Pixel color);

// Рисует отрезок целочисленным Брезенхемом, thickness — толщина в пикселях
void draw_line(Image& img, int x0, int y0, int x1, int y1, Pixel color, int thickness = 1);

// Заливает треугольник с вершинами (x0,y0), (x1,y1), (x2,y2)
void fill_triangle(Image& img, int x0, int y0, int x1, int y1, int x2, int y2, Pixel color);

// Копирует квадрат с исходника в dest и поворачивает его на angle_deg (ромб на результате)
void copy_rotated_square(const Image& src, Image& dst,
                         int src_cx, int src_cy, int half_side,
                         int dst_cx, int dst_cy, double angle_deg);

// Рисует оси OX/OY со стрелками и подписями X, Y; (ox, oy) — начало координат
void draw_axes_with_arrows(Image& img, int ox, int oy, Pixel color);

// Строит график y = cos(x) через начало (ox, oy)
void plot_cos(Image& img, int ox, int oy, double px_per_unit, double amplitude, Pixel color, int thickness);
