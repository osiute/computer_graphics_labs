#pragma once
#include "image.h"

// Попиксельное преобразование по варианту (насыщенность +30% при яркости < 50%)
void apply_variant_filter(Image& img);

// Наложение одного изображения на другое с прозрачностью alpha [0.0, 1.0]
Image blend_images(const Image& base, const Image& overlay, double alpha);