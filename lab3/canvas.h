#pragma once
#include <vector>
#include <string>

using Canvas = std::vector<std::vector<int>>;

Canvas create_canvas(int width, int height);

void save_canvas_to_pbm(const std::string& filename, const Canvas& canvas);

// Нарисовать разделительные линии крест-накрест (для визуализации 4 квадрантов)
void draw_grid(Canvas& canvas, int cx, int cy);