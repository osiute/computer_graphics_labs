#include "lab14.h"
#include <cmath>
#include <algorithm>
#include <limits>

static const double PI = 3.14159265358979323846;
static const double kXMin = 0.0;
static const double kXMax = 2.0 * PI;
static const double kZMin = 0.0;
static const double kZMax = 2.0 * PI;
static const double kYBoost = 3.2;
static const double kXzScale = 52.0;

double surface_y(double x, double z) {
    // П. 9г: y = -2 sin((π/4 - 2x + 4z) / 2) sin(x²), x,z ∈ [0, 2π]
    double inner = (PI / 4.0 - 2.0 * x + 4.0 * z) / 2.0;
    return -2.0 * std::sin(inner) * std::sin(x * x);
}

static void view_rotate(double x, double y, double z, const ViewParams& view,
                        double& ox, double& oy, double& oz) {
    double xc = x - PI;
    double zc = z - PI;
    double yc = y * kYBoost;

    double az = view.azimuth_deg * PI / 180.0;
    double el = view.elevation_deg * PI / 180.0;
    double caz = std::cos(-az);
    double saz = std::sin(-az);
    double xr = xc * caz - zc * saz;
    double zr = xc * saz + zc * caz;

    double cel = std::cos(-el);
    double sel = std::sin(-el);
    ox = xr;
    oy = yc * cel - zr * sel;
    oz = yc * sel + zr * cel;
}

ScreenPt project_point(double x, double y, double z, const ViewParams& view) {
    double ox, oy, oz;
    view_rotate(x, y, z, view, ox, oy, oz);
    double s = kXzScale * view.scene_scale;
    int sx = static_cast<int>(std::round(view.width * 0.5 + s * ox));
    int sy = static_cast<int>(std::round(view.height * 0.5 - s * oy));
    ScreenPt p;
    p.x = sx;
    p.y = sy;
    p.ok = (sx >= 0 && sx < view.width && sy >= 0 && sy < view.height);
    return p;
}

void draw_axes_grid(Image& img, const ViewParams& view) {
    Pixel grid{190, 190, 190};
    Pixel xcol{200, 40, 40};
    Pixel ycol{40, 160, 40};
    Pixel zcol{40, 80, 200};

    const int n = 8;
    for (int i = 0; i <= n; ++i) {
        double t = kXMin + (kXMax - kXMin) * i / n;
        ScreenPt a = project_point(t, 0.0, kZMin, view);
        ScreenPt b = project_point(t, 0.0, kZMax, view);
        ScreenPt c = project_point(kXMin, 0.0, t, view);
        ScreenPt d = project_point(kXMax, 0.0, t, view);
        draw_line(img, a.x, a.y, b.x, b.y, grid);
        draw_line(img, c.x, c.y, d.x, d.y, grid);
    }

    ScreenPt o = project_point(PI, 0.0, PI, view);
    ScreenPt px = project_point(PI + 1.6, 0.0, PI, view);
    ScreenPt py = project_point(PI, 1.6 / kYBoost, PI, view);
    ScreenPt pz = project_point(PI, 0.0, PI + 1.6, view);
    draw_line(img, o.x, o.y, px.x, px.y, xcol);
    draw_line(img, o.x, o.y, py.x, py.y, ycol);
    draw_line(img, o.x, o.y, pz.x, pz.y, zcol);
}

void draw_cutting_planes(Image& img, const std::vector<double>& z_planes, const ViewParams& view) {
    Pixel col{170, 140, 210};
    for (double z : z_planes) {
        ScreenPt a = project_point(kXMin, 0.0, z, view);
        ScreenPt b = project_point(kXMax, 0.0, z, view);
        draw_line(img, a.x, a.y, b.x, b.y, col);
    }
}

enum VisKind { VIS_HIDDEN = 0, VIS_ABOVE = 1, VIS_BELOW = 2 };

static VisKind classify(int x, double y, const HorizonState& h, int width) {
    if (x < 0 || x >= width) {
        return VIS_HIDDEN;
    }
    if (y < h.upper[x] - 0.01) {
        return VIS_ABOVE;
    }
    if (y > h.lower[x] + 0.01) {
        return VIS_BELOW;
    }
    return VIS_HIDDEN;
}

static void plot(Image& img, int x, int y, const Pixel& color) {
    if (x >= 0 && x < img.width && y >= 0 && y < img.height) {
        img.pixels[y * img.width + x] = color;
    }
}

// Брезенхем по отрезку: проверка видимости в каждом пикселе + запись в горизонты.
static void process_segment(Image& img, HorizonState& h, int x0, double y0, int x1, double y1,
                            const Pixel& color, bool draw_visible) {
    int iy0 = static_cast<int>(std::round(y0));
    int iy1 = static_cast<int>(std::round(y1));
    int dx = std::abs(x1 - x0);
    int dy = std::abs(iy1 - iy0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (iy0 < iy1) ? 1 : -1;
    int err = dx - dy;
    int x = x0;
    int y = iy0;
    while (true) {
        if (x >= 0 && x < img.width) {
            VisKind v = classify(x, static_cast<double>(y), h, img.width);
            if (draw_visible && v != VIS_HIDDEN) {
                plot(img, x, y, color);
            }
            h.upper[x] = std::min(h.upper[x], static_cast<double>(y));
            h.lower[x] = std::max(h.lower[x], static_cast<double>(y));
        }
        if (x == x1 && y == iy1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void floating_horizon(Image& img, const std::vector<double>& z_planes,
                      const ViewParams& view, HorizonState* last_horizons) {
    HorizonState h;
    h.upper.assign(img.width, std::numeric_limits<double>::infinity());
    h.lower.assign(img.width, -std::numeric_limits<double>::infinity());

    Pixel vis_color{25, 70, 170};
    const int n_x = img.width * 3;

    ScreenPt prev_left;
    ScreenPt prev_right;
    bool has_prev = false;

    for (size_t iz = 0; iz < z_planes.size(); ++iz) {
        double z = z_planes[iz];
        std::vector<ScreenPt> samples(n_x);
        for (int i = 0; i < n_x; ++i) {
            double x = kXMin + (kXMax - kXMin) * i / (n_x - 1);
            double y = surface_y(x, z);
            samples[i] = project_point(x, y, z, view);
        }

        ScreenPt left = samples.front();
        ScreenPt right = samples.back();

        if (has_prev) {
            process_segment(img, h, prev_left.x, prev_left.y, left.x, left.y, vis_color, true);
        }

        for (int i = 1; i < n_x; ++i) {
            process_segment(img, h, samples[i - 1].x, samples[i - 1].y,
                            samples[i].x, samples[i].y, vis_color, true);
        }

        if (has_prev) {
            process_segment(img, h, prev_right.x, prev_right.y, right.x, right.y, vis_color, true);
        }

        prev_left = left;
        prev_right = right;
        has_prev = true;
    }

    if (last_horizons) {
        *last_horizons = h;
    }
}

void draw_horizon_polylines(Image& img, const HorizonState& h, const Pixel& up_color, const Pixel& lo_color) {
    int last_ux = -1, last_uy = 0;
    int last_lx = -1, last_ly = 0;
    for (int x = 0; x < img.width; ++x) {
        if (h.upper[x] < 1e12) {
            int y = static_cast<int>(std::round(h.upper[x]));
            if (last_ux >= 0) {
                draw_line(img, last_ux, last_uy, x, y, up_color);
            }
            last_ux = x;
            last_uy = y;
        }
        if (h.lower[x] > -1e12) {
            int y = static_cast<int>(std::round(h.lower[x]));
            if (last_lx >= 0) {
                draw_line(img, last_lx, last_ly, x, y, lo_color);
            }
            last_lx = x;
            last_ly = y;
        }
    }
}
