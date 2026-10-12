#include "lab2_ops.h"
#include "netpbm.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb_image.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

Image create_blank(int width, int height, Pixel fill) {
    Image img;
    img.width = width;
    img.height = height;
    img.pixels.assign(static_cast<size_t>(width) * height, fill);
    return img;
}

Image load_image(const std::string& filename) {
    auto ends_with = [](const std::string& s, const std::string& suf) {
        if (s.size() < suf.size()) return false;
        for (size_t i = 0; i < suf.size(); ++i) {
            char a = static_cast<char>(std::tolower(static_cast<unsigned char>(s[s.size() - suf.size() + i])));
            char b = static_cast<char>(std::tolower(static_cast<unsigned char>(suf[i])));
            if (a != b) return false;
        }
        return true;
    };

    if (ends_with(filename, ".ppm")) {
        return read_ppm_p3(filename);
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
    if (data == nullptr) {
        throw std::runtime_error("Не удалось открыть изображение: " + filename);
    }

    Image img;
    img.width = width;
    img.height = height;
    img.pixels.resize(static_cast<size_t>(width) * height);
    for (int i = 0; i < width * height; ++i) {
        img.pixels[i] = {data[i * 3 + 0], data[i * 3 + 1], data[i * 3 + 2]};
    }
    stbi_image_free(data);
    return img;
}

void set_pixel(Image& img, int x, int y, Pixel color) {
    if (x < 0 || y < 0 || x >= img.width || y >= img.height) {
        return;
    }
    img.pixels[y * img.width + x] = color;
}

void draw_line(Image& img, int x0, int y0, int x1, int y1, Pixel color, int thickness) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    bool steep = dy > dx;
    if (steep) {
        std::swap(dx, dy);
    }

    int err = 2 * dy - dx;
    int x = x0;
    int y = y0;
    int radius = std::max(0, thickness / 2);

    for (int i = 0; i <= dx; ++i) {
        if (radius <= 0) {
            set_pixel(img, x, y, color);
        } else {
            for (int ty = -radius; ty <= radius; ++ty) {
                for (int tx = -radius; tx <= radius; ++tx) {
                    if (tx * tx + ty * ty <= radius * radius) {
                        set_pixel(img, x + tx, y + ty, color);
                    }
                }
            }
        }

        if (err >= 0) {
            if (steep) {
                x += sx;
            } else {
                y += sy;
            }
            err -= 2 * dx;
        }

        if (steep) {
            y += sy;
        } else {
            x += sx;
        }
        err += 2 * dy;
    }
}

void fill_triangle(Image& img, int x0, int y0, int x1, int y1, int x2, int y2, Pixel color) {
    int min_x = std::max(0, std::min({x0, x1, x2}));
    int max_x = std::min(img.width - 1, std::max({x0, x1, x2}));
    int min_y = std::max(0, std::min({y0, y1, y2}));
    int max_y = std::min(img.height - 1, std::max({y0, y1, y2}));

    auto edge = [](int ax, int ay, int bx, int by, int px, int py) {
        return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
    };

    int area = edge(x0, y0, x1, y1, x2, y2);
    if (area == 0) {
        return;
    }

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            int w0 = edge(x1, y1, x2, y2, x, y);
            int w1 = edge(x2, y2, x0, y0, x, y);
            int w2 = edge(x0, y0, x1, y1, x, y);
            bool has_neg = (w0 < 0) || (w1 < 0) || (w2 < 0);
            bool has_pos = (w0 > 0) || (w1 > 0) || (w2 > 0);
            if (!(has_neg && has_pos)) {
                set_pixel(img, x, y, color);
            }
        }
    }
}

// Рисует растровую букву 5x7 из шаблона rows
static void draw_glyph(Image& img, int left, int top, const char* rows[7], Pixel color) {
    const int scale = 2;
    for (int gy = 0; gy < 7; ++gy) {
        for (int gx = 0; gx < 5; ++gx) {
            if (rows[gy][gx] == '#') {
                for (int sy = 0; sy < scale; ++sy) {
                    for (int sx = 0; sx < scale; ++sx) {
                        set_pixel(img, left + gx * scale + sx, top + gy * scale + sy, color);
                    }
                }
            }
        }
    }
}

// Рисует подпись «X»
static void draw_label_x(Image& img, int left, int top, Pixel color) {
    const char* rows[7] = {
        "#   #",
        "#   #",
        " # # ",
        "  #  ",
        " # # ",
        "#   #",
        "#   #"
    };
    draw_glyph(img, left, top, rows, color);
}

// Рисует подпись «Y»
static void draw_label_y(Image& img, int left, int top, Pixel color) {
    const char* rows[7] = {
        "#   #",
        "#   #",
        " # # ",
        "  #  ",
        "  #  ",
        "  #  ",
        "  #  "
    };
    draw_glyph(img, left, top, rows, color);
}

void copy_rotated_square(const Image& src, Image& dst,
                         int src_cx, int src_cy, int half_side,
                         int dst_cx, int dst_cy, double angle_deg) {
    const double rad = angle_deg * 3.141592653589793 / 180.0;
    const double c = std::cos(rad);
    const double s = std::sin(rad);
    const int bound = static_cast<int>(std::ceil(half_side * 1.5));

    for (int dy = -bound; dy <= bound; ++dy) {
        for (int dx = -bound; dx <= bound; ++dx) {
            double lx =  dx * c + dy * s;
            double ly = -dx * s + dy * c;
            if (std::abs(lx) > half_side || std::abs(ly) > half_side) {
                continue;
            }

            int sx = src_cx + static_cast<int>(std::round(lx));
            int sy = src_cy + static_cast<int>(std::round(ly));
            if (sx < 0 || sy < 0 || sx >= src.width || sy >= src.height) {
                continue;
            }
            set_pixel(dst, dst_cx + dx, dst_cy + dy, src.pixels[sy * src.width + sx]);
        }
    }
}

void draw_axes_with_arrows(Image& img, int ox, int oy, Pixel color) {
    const int margin = 10;
    const int arrow_len = std::max(48, std::min(img.width, img.height) / 7);
    const int arrow_half = std::max(12, arrow_len / 5);

    int x_end = img.width - margin;
    int y_end = margin;
    int y_neg = std::min(img.height - margin, oy + std::max(28, img.height / 12));

    draw_line(img, 0, oy, x_end, oy, color, 1);
    draw_line(img, ox, y_neg, ox, y_end, color, 1);

    fill_triangle(img,
                  x_end, oy,
                  x_end - arrow_len, oy - arrow_half,
                  x_end - arrow_len, oy + arrow_half,
                  color);
    fill_triangle(img,
                  ox, y_end,
                  ox - arrow_half, y_end + arrow_len,
                  ox + arrow_half, y_end + arrow_len,
                  color);

    draw_label_x(img, x_end - 18, oy + arrow_half + 6, color);
    draw_label_y(img, ox + arrow_half + 4, y_end + 8, color);
}

void plot_cos(Image& img, int ox, int oy, double px_per_unit, double amplitude, Pixel color, int thickness) {
    int prev_x = 0;
    int prev_y = 0;
    bool has_prev = false;

    for (int x = 0; x < img.width; ++x) {
        double t = (x - ox) / px_per_unit;
        int y = static_cast<int>(std::round(oy - amplitude * std::cos(t)));
        if (has_prev) {
            draw_line(img, prev_x, prev_y, x, y, color, thickness);
        }
        prev_x = x;
        prev_y = y;
        has_prev = true;
    }
}
