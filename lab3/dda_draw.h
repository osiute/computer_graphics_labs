#pragma once

#include <vector>
#include "geometry.h"
#include "canvas.h"

// Отрисовка отрезков по алгоритму ЦДА (Цифровой дифференциальный анализатор)
void dda_draw_segments(Canvas& canvas, const std::string& filename, int x_offset, int y_offset);