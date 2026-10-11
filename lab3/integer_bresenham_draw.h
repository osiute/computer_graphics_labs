#pragma once

#include <vector>
#include "geometry.h"
#include "canvas.h"

// Отрисовка отрезков по целочисленному алгоритму Брезенхема (без деления и float)
void integer_bresenham_draw_segments(Canvas& canvas, const std::string& filename, int x_offset, int y_offset);