#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif

#include "image.h"
#include "netpbm.h"
#include "geometry.h"
#include "surfaces.h"
#include "curves.h"
#include "obj_io.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libraries/stb_image_write.h"

static Image make_canvas(int w, int h) {
    Image img;
    img.width = w;
    img.height = h;
    img.pixels.assign(static_cast<size_t>(w) * h, Pixel{255, 255, 255});
    return img;
}

static void save_png(const std::string& filename, const Image& img) {
    std::vector<unsigned char> rgb(static_cast<size_t>(img.width) * img.height * 3);
    for (int i = 0; i < img.width * img.height; ++i) {
        rgb[i * 3 + 0] = img.pixels[i].r;
        rgb[i * 3 + 1] = img.pixels[i].g;
        rgb[i * 3 + 2] = img.pixels[i].b;
    }
    stbi_write_png(filename.c_str(), img.width, img.height, 3, rgb.data(), img.width * 3);
}

static std::string stem_of(const std::string& filename) {
    std::string s = filename;
    auto dot = s.find_last_of('.');
    if (dot != std::string::npos) {
        s = s.substr(0, dot);
    }
    return s;
}

static void save_raster(const std::string& filename, const Image& img) {
    write_ppm_p3(filename, img);
    save_png(stem_of(filename) + ".png", img);
    std::cout << "Растр: " << filename << std::endl;
}

static void save_mesh_obj(const std::string& filename, const std::string& tag, const Mesh& mesh) {
    std::vector<ObjVert> verts;
    for (const auto& p : mesh.vertices) {
        verts.push_back({p.x, p.y, p.z});
    }
    std::vector<ObjFace> faces;
    for (const auto& f : mesh.faces) {
        faces.push_back({f.vertex_indices});
    }
    std::string path = stem_of(filename) + ".obj";
    if (write_wavefront_obj(path, tag, verts, faces)) {
        std::cout << "OBJ: " << path << std::endl;
    }
}

static void draw_polyline(Image& img, const std::vector<Point2D>& pts, const Pixel& color) {
    for (size_t i = 0; i + 1 < pts.size(); ++i) {
        draw_line(img,
                  static_cast<int>(pts[i].x), static_cast<int>(pts[i].y),
                  static_cast<int>(pts[i + 1].x), static_cast<int>(pts[i + 1].y), color);
    }
}

static void draw_controls(Image& img, const std::vector<Point2D>& pts, const Pixel& line, const Pixel& dot) {
    draw_polyline(img, pts, line);
    for (const auto& p : pts) {
        draw_point(img, static_cast<int>(p.x), static_cast<int>(p.y), 5, dot);
    }
}

static std::vector<Point2D> read_points2d(int minimum) {
    int n = 0;
    std::cout << "Число вершин (0 = заготовка, минимум " << minimum << "): ";
    std::cin >> n;
    std::vector<Point2D> pts;
    if (n <= 0) {
        return pts;
    }
    if (n < minimum) {
        n = minimum;
    }
    for (int i = 0; i < n; ++i) {
        Point2D p;
        std::cout << "Вершина " << (i + 1) << " (x y): ";
        std::cin >> p.x >> p.y;
        pts.push_back(p);
    }
    return pts;
}

static void run_cubic(const std::string& filename, std::vector<Point2D> pts, int steps, double t_show) {
    if (pts.empty()) {
        pts = {
            {80, 500}, {180, 80}, {360, 80}, {400, 340},
            {440, 80}, {620, 80}, {720, 500}
        };
        std::cout << "Заготовка: 7 точек, два кубических сегмента." << std::endl;
    }
    if (steps < 2) {
        steps = 2;
    }

    Image img = make_canvas(800, 600);
    draw_controls(img, pts, Pixel{180, 180, 180}, Pixel{30, 80, 180});

    std::vector<Point2D> first4(pts.begin(), pts.begin() + (pts.size() >= 4 ? 4 : pts.size()));
    auto layers = decasteljau_layers(first4, t_show);
    Pixel layer_colors[4] = {{40, 160, 70}, {200, 140, 20}, {160, 40, 160}, {0, 0, 0}};
    std::cout << "Промежуточные вершины де Кастельжо при t=" << t_show << ":\n";
    for (size_t L = 1; L < layers.size(); ++L) {
        std::cout << "  слой " << L << ":";
        for (const auto& p : layers[L]) {
            std::cout << " (" << std::fixed << std::setprecision(1) << p.x << "," << p.y << ")";
            draw_point(img, static_cast<int>(p.x), static_cast<int>(p.y), 4, layer_colors[(L - 1) % 4]);
        }
        std::cout << "\n";
        draw_polyline(img, layers[L], layer_colors[(L - 1) % 4]);
    }

    std::vector<Point2D> curve = generate_cubic_bezier(pts, steps);
    draw_polyline(img, curve, Pixel{220, 20, 20});
    for (const auto& p : curve) {
        draw_point(img, static_cast<int>(p.x), static_cast<int>(p.y), 1, Pixel{180, 20, 20});
    }
    save_raster(filename, img);
}

static void run_chaikin(const std::string& filename, std::vector<Point2D> pts, int iterations) {
    if (pts.empty()) {
        pts = {{100, 420}, {220, 120}, {400, 80}, {600, 160}, {700, 460}, {430, 530}};
        std::cout << "Заготовка ломаной Чайкина: 6 вершин." << std::endl;
    }
    if (iterations < 1) {
        iterations = 1;
    }
    Image img = make_canvas(800, 600);
    draw_controls(img, pts, Pixel{180, 180, 180}, Pixel{30, 80, 180});
    std::vector<Point2D> refined = generate_chaikin(pts, iterations);
    draw_polyline(img, refined, Pixel{220, 20, 20});
    for (const auto& p : refined) {
        draw_point(img, static_cast<int>(p.x), static_cast<int>(p.y), 2, Pixel{200, 40, 40});
    }
    std::cout << "Исходных вершин: " << pts.size()
              << ", после " << iterations << " итераций: " << refined.size() << std::endl;
    save_raster(filename, img);
}

static void run_bezier_surface(const std::string& filename, int resolution) {
    if (resolution < 2) {
        resolution = 2;
    }
    std::vector<std::vector<Point3D>> control_grid = {
        {{-140, -120, -50}, {-60, -120, -20}, {20, -120, 20}, {100, -120, 50}},
        {{-140, -40, -20}, {-60, -40, 50}, {20, -40, 90}, {100, -40, 120}},
        {{-140, 40, -20}, {-60, 40, 50}, {20, 40, 90}, {100, 40, 120}},
        {{-140, 120, -50}, {-60, 120, -20}, {20, 120, 20}, {100, 120, 50}}
    };
    Mesh surface = generate_bezier_surface(control_grid, resolution, resolution);
    Mesh control;
    for (const auto& row : control_grid) {
        for (const auto& pt : row) {
            control.vertices.push_back(pt);
        }
    }
    for (int i = 0; i < 4; ++i) {
        control.faces.push_back({{i * 4, i * 4 + 1, i * 4 + 2, i * 4 + 3}});
        control.faces.push_back({{i, i + 4, i + 8, i + 12}});
    }

    Image img = make_canvas(800, 600);
    draw_mesh(img, surface, 450.0, Pixel{0, 130, 0}, Pixel{200, 30, 30});
    draw_mesh(img, control, 450.0, Pixel{100, 150, 255}, Pixel{220, 20, 20});
    save_raster(filename, img);
    save_mesh_obj(filename, "BezierSurface", surface);
}

static void run_doo_sabin(const std::string& filename, int iterations) {
    if (iterations < 1) {
        iterations = 1;
    }
    double s = 100.0;
    Mesh cube;
    cube.vertices = {
        {-s, -s, -s}, {s, -s, -s}, {s, s, -s}, {-s, s, -s},
        {-s, -s, s}, {s, -s, s}, {s, s, s}, {-s, s, s}
    };
    cube.faces = {
        {{0, 3, 2, 1}}, {{4, 5, 6, 7}}, {{0, 1, 5, 4}},
        {{2, 3, 7, 6}}, {{0, 4, 7, 3}}, {{1, 2, 6, 5}}
    };
    Mesh subdivided = subdivide_doo_sabin(cube, iterations);
    std::cout << "Ду-Сабин: исходных вершин " << cube.vertices.size()
              << ", после " << iterations << " итераций: " << subdivided.vertices.size()
              << ", граней: " << subdivided.faces.size() << std::endl;

    Image img = make_canvas(800, 600);
    draw_mesh(img, subdivided, 480.0, Pixel{0, 130, 0}, Pixel{200, 30, 30});
    draw_mesh(img, cube, 480.0, Pixel{200, 50, 50}, Pixel{220, 20, 20});
    save_raster(filename, img);
    save_mesh_obj(filename, "DooSabin", subdivided);
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Лаба 12, вариант 2 (m2664) ===" << std::endl;
    std::cout << "П.6 обязательно. Ещё: кубическая Безье, Чайкин, поверхность Безье, Ду-Сабин.\n"
              << "Серый — исходный полигон, синие точки — его вершины,\n"
              << "красный/зелёный — уточнённая кривая или поверхность.\n" << std::endl;
    std::cout << "Режим:\n"
              << "  2 - кубическая Безье\n"
              << "  3 - Чайкин\n"
              << "  4 - поверхность Безье\n"
              << "  5 - поверхность Ду-Сабина\n"
              << "  0 - все четыре заготовки\n"
              << "Выбор: ";
    int mode = 0;
    std::cin >> mode;

    if (mode == 2) {
        auto pts = read_points2d(4);
        int steps = 40;
        std::cout << "Параметр разбиения (число шагов, например 40): ";
        std::cin >> steps;
        std::string fn;
        std::cout << "Имя PPM: ";
        std::cin >> fn;
        run_cubic(fn, pts, steps, 0.5);
    } else if (mode == 3) {
        auto pts = read_points2d(3);
        int it = 4;
        std::cout << "Число итераций Чайкина (например 4): ";
        std::cin >> it;
        std::string fn;
        std::cout << "Имя PPM: ";
        std::cin >> fn;
        run_chaikin(fn, pts, it);
    } else if (mode == 4) {
        int res = 12;
        std::cout << "Параметр разбиения поверхности (например 12): ";
        std::cin >> res;
        std::string fn;
        std::cout << "Имя PPM: ";
        std::cin >> fn;
        run_bezier_surface(fn, res);
    } else if (mode == 5) {
        int it = 2;
        std::cout << "Итерации Ду-Сабина (например 2): ";
        std::cin >> it;
        std::string fn;
        std::cout << "Имя PPM: ";
        std::cin >> fn;
        run_doo_sabin(fn, it);
    } else {
        run_cubic("cubic_bezier.ppm", {}, 50, 0.5);
        run_chaikin("chaikin.ppm", {}, 4);
        run_bezier_surface("bezier.ppm", 12);
        run_doo_sabin("doo_sabin.ppm", 2);
    }
    return 0;
}
