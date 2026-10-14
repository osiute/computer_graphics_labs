#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#endif

#include "image.h"
#include "netpbm.h"
#include "lab14.h"
#include "obj_io.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libraries/stb_image_write.h"

static const double PI = 3.14159265358979323846;

static void save_png(const std::string& filename, const Image& img) {
    std::vector<unsigned char> rgb(static_cast<size_t>(img.width) * img.height * 3);
    for (int i = 0; i < img.width * img.height; ++i) {
        rgb[i * 3 + 0] = img.pixels[i].r;
        rgb[i * 3 + 1] = img.pixels[i].g;
        rgb[i * 3 + 2] = img.pixels[i].b;
    }
    stbi_write_png(filename.c_str(), img.width, img.height, 3, rgb.data(), img.width * 3);
}

static Image make_canvas(int w, int h) {
    Image img;
    img.width = w;
    img.height = h;
    img.pixels.assign(static_cast<size_t>(w) * h, Pixel{255, 255, 255});
    return img;
}

static void save_both(const std::string& stem, const Image& img) {
    write_ppm_p3(stem + ".ppm", img);
    save_png(stem + ".png", img);
}

static void setup_camera(ViewParams& view, const Point3D& eye) {
    view.eye = eye;
    view.look_at = {0.0, 50.0, 0.0};
    view.azimuth_deg = std::atan2(eye.x - view.look_at.x, eye.z - view.look_at.z) * 180.0 / PI;
    double dx = eye.x - view.look_at.x;
    double dz = eye.z - view.look_at.z;
    double r = std::sqrt(dx * dx + dz * dz);
    view.elevation_deg = std::atan2(eye.y - view.look_at.y, r) * 180.0 / PI;
}

static void print_plane(std::ostream& out, const Plane& p) {
    out << std::fixed << std::setprecision(4)
        << std::setw(10) << p.a << " "
        << std::setw(10) << p.b << " "
        << std::setw(10) << p.c << " "
        << std::setw(10) << p.d;
}

static void dump_body_matrix(std::ostream& out, const std::vector<Plane>& body) {
    out << "Матрица тела V (столбец = грань, строки a b c d, знаки уже проверены)\n";
    const char* row_name[4] = {"a", "b", "c", "d"};
    for (int row = 0; row < 4; ++row) {
        out << row_name[row] << " |";
        for (const auto& col : body) {
            double v = (row == 0) ? col.a : (row == 1) ? col.b : (row == 2) ? col.c : col.d;
            out << std::setw(12) << std::fixed << std::setprecision(3) << v;
        }
        out << "\n";
    }
}

static void render_solid(const Mesh& mesh, const std::string& name,
                         const ViewParams& view, const Point3D& eye) {
    Point3D interior = mesh_interior_point(mesh);
    std::vector<Plane> body = build_body_matrix(mesh);
    std::vector<bool> back(body.size(), false);
    for (size_t f = 0; f < body.size(); ++f) {
        back[f] = is_back_face(body[f], eye);
    }

    Image img = make_canvas(view.width, view.height);
    draw_axes_grid_solid(img, view);
    draw_mesh_roberts(img, mesh, back, view);
    save_both(name, img);

    {
        std::vector<ObjVert> verts;
        for (const auto& p : mesh.vertices) {
            verts.push_back({p.x, p.y, p.z});
        }
        std::vector<ObjFace> faces;
        for (const auto& f : mesh.faces) {
            faces.push_back({f.vertex_indices});
        }
        std::string obj_path = name + ".obj";
        if (write_wavefront_obj(obj_path, name, verts, faces)) {
            std::cout << "OBJ сохранён: " << obj_path << std::endl;
        }
    }

    std::ofstream report(name + "_roberts.txt");
    auto dump = [&](const std::string& s) {
        std::cout << s;
        report << s;
    };

    dump("=== " + name + " ===\n");
    std::cout << "вершин " << mesh.vertices.size() << ", граней " << mesh.faces.size() << "\n";
    report << "вершин " << mesh.vertices.size() << ", граней " << mesh.faces.size() << "\n";
    std::cout << "внутри " << interior.x << " " << interior.y << " " << interior.z << "\n";
    report << "внутри " << interior.x << " " << interior.y << " " << interior.z << "\n";
    std::cout << "точка наблюдения " << eye.x << " " << eye.y << " " << eye.z << "\n\n";
    report << "точка наблюдения " << eye.x << " " << eye.y << " " << eye.z << "\n\n";

    dump("Координаты вершин (как заданы):\n");
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        const Point3D& v = mesh.vertices[i];
        std::cout << "  v" << (i + 1) << "  " << v.x << " " << v.y << " " << v.z << "\n";
        report << "  v" << (i + 1) << "  " << v.x << " " << v.y << " " << v.z << "\n";
    }

    dump("\nТри способа уравнения плоскости (до правки знака)\n");
    for (size_t f = 0; f < mesh.faces.size(); ++f) {
        PlaneTriple t = compute_face_three_ways(mesh, mesh.faces[f]);
        std::cout << "Грань " << (f + 1) << ":\n  способ 1 (d=1 / запасная норма): ";
        report << "Грань " << (f + 1) << ":\n  способ 1 (d=1 / запасная норма): ";
        if (t.method1_ok) {
            print_plane(std::cout, t.method1);
            print_plane(report, t.method1);
            std::cout << "  [" << t.method1_note << "]";
            report << "  [" << t.method1_note << "]";
        } else {
            std::cout << "не применим (" << t.method1_note << ")";
            report << "не применим (" << t.method1_note << ")";
        }
        std::cout << "\n  способ 2 (нормаль): ";
        report << "\n  способ 2 (нормаль): ";
        print_plane(std::cout, t.method2);
        print_plane(report, t.method2);
        std::cout << "\n  способ 3 (Ньюэл):   ";
        report << "\n  способ 3 (Ньюэл):   ";
        print_plane(std::cout, t.method3);
        print_plane(report, t.method3);
        double s = homogeneous_dot(body[f], interior.x, interior.y, interior.z, 1.0);
        std::cout << "\n  знак после проверки interior·P = " << s
                  << (back[f] ? "  НЕЛИЦЕВАЯ из точки наблюдения\n" : "  лицевая из точки наблюдения\n");
        report << "\n  знак после проверки interior·P = " << s
               << (back[f] ? "  НЕЛИЦЕВАЯ из точки наблюдения\n" : "  лицевая из точки наблюдения\n");
    }

    dump("\n");
    dump_body_matrix(std::cout, body);
    dump_body_matrix(report, body);
    report.close();
    std::cout << "Картинка: " << name << ".png  (синие лицевые, серый пунктир нелицевые)\n" << std::endl;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Лаба 14, вариант 2: этап 1 алгоритма Робертса ===" << std::endl;
    std::cout << "а) матрица тела по вершинам\n"
              << "б) три способа плоскости + проверка знаков\n"
              << "в) нелицевые грани из точки наблюдения\n"
              << "Объекты: параллелепипед, призма, произвольный (домик).\n" << std::endl;

    std::cout << "Объект:\n"
              << "  1 - параллелепипед (заготовка)\n"
              << "  2 - призма (заготовка)\n"
              << "  3 - произвольный: домик или свои вершины\n"
              << "  0 - все три заготовки (для отчёта)\n"
              << "Выбор: ";
    int choice = 0;
    std::cin >> choice;

    Point3D eye{220.0, 190.0, 280.0};
    std::cout << "Точка наблюдения x y z (например 220 190 280): ";
    std::cin >> eye.x >> eye.y >> eye.z;

    ViewParams view;
    view.width = 900;
    view.height = 700;
    view.scene_scale = 1.15;
    setup_camera(view, eye);

    std::cout << "Масштаб сцены (например 1.15): ";
    std::cin >> view.scene_scale;
    if (view.scene_scale <= 0.0) {
        view.scene_scale = 1.0;
    }

    std::cout << "Камера из точки наблюдения: азимут "
              << view.azimuth_deg << " град, место "
              << view.elevation_deg << " град\n" << std::endl;

    if (choice == 1) {
        render_solid(create_parallelepiped(80.0, 80.0, 40.0), "parallelepiped", view, eye);
    } else if (choice == 2) {
        render_solid(create_triangular_prism(80.0, 80.0), "prism", view, eye);
    } else if (choice == 3) {
        render_solid(load_mesh_from_stdin(), "house", view, eye);
    } else {
        render_solid(create_parallelepiped(80.0, 80.0, 40.0), "parallelepiped", view, eye);
        render_solid(create_triangular_prism(80.0, 80.0), "prism", view, eye);
        render_solid(create_house(), "house", view, eye);
    }

    return 0;
}
