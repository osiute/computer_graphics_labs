#pragma once
#include "image.h"
#include <string>

enum InterpMethod {
    INTERP_NEAREST = 0,
    INTERP_BILINEAR = 1,
    INTERP_BICUBIC = 2
};

// Загружает изображение из файла (PPM / JPG / PNG)
Image load_image(const std::string& filename);

// Сохраняет изображение в PNG
void save_png(const std::string& filename, const Image& img);

// Создаёт изображение заданного размера и заливает цветом
Image create_blank(int width, int height, Pixel fill);

// Берёт яркость исходника в (x, y) интерполяцией nearest / bilinear / bicubic
Pixel interpolate(const Image& src, double x, double y, InterpMethod method, Pixel outside);

// Масштабирует изображение: i = sx*x', j = sy*y'
Image affine_scale(const Image& src, double sx, double sy, InterpMethod method);

// Поворачивает изображение вокруг центра на угол в градусах
Image affine_rotate(const Image& src, double angle_deg, InterpMethod method);

// Делает скос по X вокруг центра: i = x' + sh*y', j = y'
Image affine_shear_x(const Image& src, double shear, InterpMethod method);
