#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
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

static std::vector<std::vector<Point3D>> default_bezier_grid() {
    return {
        {{-140,-120,-50},{-60,-120,-20},{20,-120,20},{100,-120,50}},
        {{-140,-40,-20},{-60,-40,50},{20,-40,90},{100,-40,120}},
        {{-140,40,-20},{-60,40,50},{20,40,90},{100,40,120}},
        {{-140,120,-50},{-60,120,-20},{20,120,20},{100,120,50}}
    };
}

static std::vector<std::vector<Point3D>> input_bezier_grid() {
    int rows;
    std::cout << "Строк контрольной сетки (0 = пример 4x4): ";
    std::cin >> rows;
    if (rows == 0) return default_bezier_grid();
    int cols;
    std::cout << "Столбцов: ";
    std::cin >> cols;
    if (rows < 2 || cols < 2 || rows > 30 || cols > 30)
        throw std::invalid_argument("Размеры сетки: от 2 до 30 по каждой оси");
    std::vector<std::vector<Point3D>> grid(rows, std::vector<Point3D>(cols));
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            std::cout << "P[" << r << "][" << c << "] (x y z): ";
            if (!(std::cin >> grid[r][c].x >> grid[r][c].y >> grid[r][c].z))
                throw std::runtime_error("Некорректные координаты");
        }
    return grid;
}

static void draw_bezier_controls(Image& img, const std::vector<std::vector<Point3D>>& grid) {
    int rows = static_cast<int>(grid.size()), cols = static_cast<int>(grid[0].size());
    auto line3 = [&](const Point3D& a, const Point3D& b) {
        auto x = apply_perspective(a, 450, img.width / 2.0, img.height / 2.0);
        auto y = apply_perspective(b, 450, img.width / 2.0, img.height / 2.0);
        draw_line(img, static_cast<int>(x.x), static_cast<int>(x.y),
                  static_cast<int>(y.x), static_cast<int>(y.y), Pixel{80,140,240});
    };
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            if (r + 1 < rows) line3(grid[r][c], grid[r+1][c]);
            if (c + 1 < cols) line3(grid[r][c], grid[r][c+1]);
        }
    for (const auto& row : grid)
        for (const auto& p : row) {
            auto q = apply_perspective(p, 450, img.width / 2.0, img.height / 2.0);
            draw_point(img, static_cast<int>(q.x), static_cast<int>(q.y), 4, Pixel{220,30,30});
        }
}

static void run_bezier_surface(const std::string& filename, int resolution,
                               const std::vector<std::vector<Point3D>>& grid) {
    if (resolution < 2 || resolution > 200)
        throw std::invalid_argument("Разбиение поверхности: от 2 до 200");
    Mesh surface = generate_bezier_surface(grid, resolution, resolution);
    Image img = make_canvas(800, 600);
    draw_mesh(img, surface, 450.0, Pixel{0, 130, 0}, Pixel{30, 130, 30});
    draw_bezier_controls(img, grid);
    save_raster(filename, img);
    save_mesh_obj(filename, "BezierSurface", surface);
}

static Mesh default_cube() {
    double s = 100;
    Mesh cube;
    cube.vertices = {{-s,-s,-s},{s,-s,-s},{s,s,-s},{-s,s,-s},
                     {-s,-s,s},{s,-s,s},{s,s,s},{-s,s,s}};
    cube.faces = {{{0,3,2,1}},{{4,5,6,7}},{{0,1,5,4}},
                  {{2,3,7,6}},{{0,4,7,3}},{{1,2,6,5}}};
    return cube;
}

static Mesh input_doo_sabin_mesh() {
    int n;
    std::cout << "Число исходных вершин (0 = куб): ";
    std::cin >> n;
    if (n == 0) return default_cube();
    if (n < 4 || n > 10000) throw std::invalid_argument("Количество вершин: 4..10000");
    Mesh mesh;
    mesh.vertices.resize(n);
    for (int i = 0; i < n; ++i) {
        std::cout << "Вершина " << i << " (x y z): ";
        if (!(std::cin >> mesh.vertices[i].x >> mesh.vertices[i].y >> mesh.vertices[i].z))
            throw std::runtime_error("Некорректные координаты");
    }
    int face_count;
    std::cout << "Число граней: "; std::cin >> face_count;
    if (face_count < 4 || face_count > 20000) throw std::invalid_argument("Количество граней: 4..20000");
    for (int f = 0; f < face_count; ++f) {
        int k;
        std::cout << "Грань " << f << ": число вершин и затем индексы (с 0, обход согласован): ";
        std::cin >> k;
        if (k < 3 || k > n) throw std::invalid_argument("Некорректный размер грани");
        Face face;
        for (int j = 0; j < k; ++j) {
            int id; std::cin >> id;
            if (id < 0 || id >= n) throw std::invalid_argument("Некорректный индекс вершины");
            face.vertex_indices.push_back(id);
        }
        mesh.faces.push_back(face);
    }
    return mesh;
}

static void run_doo_sabin(const std::string& filename, int iterations, const Mesh& initial) {
    if (iterations < 1 || iterations > 5)
        throw std::invalid_argument("Число итераций Ду-Сабина: от 1 до 5");
    for (int stage = 1; stage <= iterations; ++stage) {
        Mesh subdivided = subdivide_doo_sabin(initial, stage);
        std::string output = stem_of(filename) + "_step" + std::to_string(stage) + ".ppm";
        std::cout << "Шаг " << stage << ": " << subdivided.vertices.size()
                  << " вершин, " << subdivided.faces.size() << " граней\n";
        Image img = make_canvas(800, 600);
        draw_mesh(img, subdivided, 480.0, Pixel{0,130,0}, Pixel{230,150,30});
        draw_mesh(img, initial, 480.0, Pixel{80,140,240}, Pixel{220,30,30});
        save_raster(output, img);
        save_mesh_obj(output, "DooSabin", subdivided);
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Лабораторная работа #12: Поверхности (Безье / Ду-Сабина) ===" << std::endl;
    int mode = 0;
    while (true) {
        std::cout << "Режим:\n"
                  << "  1 - поверхность Безье\n"
                  << "  2 - поверхность Ду-Сабина\n"
                  << "Выбор: ";
        std::cin >> mode;
    
        if (mode == 1) {
            int res = 12;
            std::cout << "Параметр разбиения поверхности (например 12): ";
            std::cin >> res;
            std::string fn;
            std::cout << "Имя PPM: ";
            std::cin >> fn;
            run_bezier_surface(fn, res, input_bezier_grid());
            break;
        } else if (mode == 2) {
            int it = 2;
            std::cout << "Итерации Ду-Сабина (например 2): ";
            std::cin >> it;
            std::string fn;
            std::cout << "Имя PPM: ";
            std::cin >> fn;
            run_doo_sabin(fn, it, input_doo_sabin_mesh());
            break;
        } 
        else {
            std::cout << "Режим не распознан" << std::endl;
        }
    }    

    return 0;
}
