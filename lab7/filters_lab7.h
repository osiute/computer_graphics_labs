#pragma once
#include "image.h"

// ФНЧ: Возвращает новое изображение с размытием гауссианом за пределами круга радиуса R
Image apply_gaussian_blur_outside_circle(const Image& img, double radius, double sigma);

// ФВЧ: Возвращает новое изображение с повышенной резскостью для пикселей с яркостью (V) > T
Image apply_sharpen_above_brightness(const Image& img, double threshold);