#pragma once
#include "image.h"
#include <string>

// Загружает изображение из файла (PPM / JPG / PNG)
Image load_image(const std::string& filename);

// Ставит пиксель в точку (x, y)
void set_pixel(Image& img, int x, int y, Pixel color);

// Рисует отрезок из (x0, y0) в (x1, y1)
void draw_line(Image& img, int x0, int y0, int x1, int y1, Pixel color);

// Ставит 4 симметричные точки окружности
void plot_quadrants(Image& img, int cx, int cy, int x, int y, Pixel color);

// Ставит 8 симметричных точек окружности
void plot_octants(Image& img, int cx, int cy, int x, int y, Pixel color);

// Рисует крест, делящий кадр на 4 части
void draw_split(Image& img, Pixel color);

// Считает радиус описанной окружности по стороне многоугольника
double circumradius_from_side(double side, int n);

// Считает радиус вписанной окружности по стороне многоугольника
double inradius_from_side(double side, int n);

// Рисует правильный пятиугольник
void draw_regular_pentagon(Image& img, int cx, int cy, int circum_r, Pixel color);

// Рисует окружность по уравнению x^2 + y^2 = R^2
void draw_circle_equation(Image& img, int cx, int cy, int radius, Pixel color);

// Рисует окружность по параметрическим уравнениям
void draw_circle_parametric(Image& img, int cx, int cy, int radius, Pixel color);

// Рисует окружность алгоритмом Брезенхема
void draw_circle_bresenham(Image& img, int cx, int cy, int radius, Pixel color);

// Рисует окружность функцией CImg::draw_circle
void draw_circle_cimg(Image& img, int cx, int cy, int radius, Pixel color);
