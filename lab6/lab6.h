#pragma once
#include "image.h"
#include <string>

// Загружает изображение из файла (PPM / JPG / PNG)
Image load_image(const std::string& filename);

// Сохраняет изображение в PNG
void save_png(const std::string& filename, const Image& img);

// Создаёт изображение заданного размера и заливает цветом
Image create_blank(int width, int height, Pixel fill);

// Берёт ближайший пиксель исходника в точке (x, y)
Pixel sample_nearest(const Image& src, double x, double y, Pixel outside);

// Ставит пиксель в точку (x, y), если она внутри кадра
void put_pixel(Image& img, int x, int y, Pixel color);

// Прямое аффинное: i = a*x' + c, j = e*y' + f (начало — левый нижний угол)
Image affine_forward(const Image& src, double a, double e, double c, double f);

// Обратное аффинное: x' = (i-c)/a, y' = (j-f)/e (без дыр)
Image affine_inverse(const Image& src, double a, double e, double c, double f);

// Восстанавливает исходник обратным аффинным преобразованием
Image affine_restore(const Image& transformed, double a, double e, double c, double f);

// Преобразует координаты по закону i = 2^{x'}, j = y'
Image functional_pow2_x(const Image& src);
