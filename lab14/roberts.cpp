#include "lab14.h"
#include <cmath>
#include <algorithm>
#include <set>
#include <string>

static const double kEps = 1e-9;

static double det3(double a11, double a12, double a13,
                   double a21, double a22, double a23,
                   double a31, double a32, double a33) {
    return a11 * (a22 * a33 - a23 * a32)
         - a12 * (a21 * a33 - a23 * a31)
         + a13 * (a21 * a32 - a22 * a31);
}

static bool solve3x3(double A[3][3], double b[3], double x[3]) {
    double d = det3(A[0][0], A[0][1], A[0][2],
                    A[1][0], A[1][1], A[1][2],
                    A[2][0], A[2][1], A[2][2]);
    if (std::fabs(d) < kEps) {
        return false;
    }
    x[0] = det3(b[0], A[0][1], A[0][2],
                b[1], A[1][1], A[1][2],
                b[2], A[2][1], A[2][2]) / d;
    x[1] = det3(A[0][0], b[0], A[0][2],
                A[1][0], b[1], A[1][2],
                A[2][0], b[2], A[2][2]) / d;
    x[2] = det3(A[0][0], A[0][1], b[0],
                A[1][0], A[1][1], b[1],
                A[2][0], A[2][1], b[2]) / d;
    return true;
}

Point3D mesh_interior_point(const Mesh& mesh) {
    Point3D c{0.0, 0.0, 0.0};
    if (mesh.vertices.empty()) {
        return c;
    }
    for (const auto& v : mesh.vertices) {
        c.x += v.x;
        c.y += v.y;
        c.z += v.z;
    }
    double n = static_cast<double>(mesh.vertices.size());
    c.x /= n;
    c.y /= n;
    c.z /= n;
    return c;
}

double homogeneous_dot(const Plane& plane, double x, double y, double z, double w) {
    return plane.a * x + plane.b * y + plane.c * z + plane.d * w;
}

static void fix_plane_sign(Plane& plane, const Point3D& interior) {
    double s = homogeneous_dot(plane, interior.x, interior.y, interior.z, 1.0);
    if (s < 0.0) {
        plane.a = -plane.a;
        plane.b = -plane.b;
        plane.c = -plane.c;
        plane.d = -plane.d;
    }
}

static Plane plane_method1(const Point3D& p1, const Point3D& p2, const Point3D& p3,
                           bool& ok, std::string& note) {
    Plane plane{0.0, 0.0, 0.0, 1.0};
    const Point3D pts[3] = {p1, p2, p3};

    // Как в методичке: нормировка d = 1, [X][a b c]^T = [-1 -1 -1]^T
    double A[3][3] = {
        {p1.x, p1.y, p1.z},
        {p2.x, p2.y, p2.z},
        {p3.x, p3.y, p3.z}
    };
    double rhs[3] = {-1.0, -1.0, -1.0};
    double x[3] = {0.0, 0.0, 0.0};
    if (solve3x3(A, rhs, x)) {
        plane.a = x[0];
        plane.b = x[1];
        plane.c = x[2];
        plane.d = 1.0;
        ok = true;
        note = "d=1";
        return plane;
    }

    // Плоскость через начало (d=0) или столбец нулей: фиксируем a=1, затем b=1, затем c=1
    auto try_fix = [&](int which, const char* label) -> bool {
        double M[3][3];
        double r[3];
        for (int i = 0; i < 3; ++i) {
            double nx = pts[i].x;
            double ny = pts[i].y;
            double nz = pts[i].z;
            if (which == 0) {
                M[i][0] = ny;
                M[i][1] = nz;
                M[i][2] = 1.0;
                r[i] = -nx;
            } else if (which == 1) {
                M[i][0] = nx;
                M[i][1] = nz;
                M[i][2] = 1.0;
                r[i] = -ny;
            } else {
                M[i][0] = nx;
                M[i][1] = ny;
                M[i][2] = 1.0;
                r[i] = -nz;
            }
        }
        double sol[3] = {0.0, 0.0, 0.0};
        if (!solve3x3(M, r, sol)) {
            return false;
        }
        if (which == 0) {
            plane.a = 1.0;
            plane.b = sol[0];
            plane.c = sol[1];
            plane.d = sol[2];
        } else if (which == 1) {
            plane.a = sol[0];
            plane.b = 1.0;
            plane.c = sol[1];
            plane.d = sol[2];
        } else {
            plane.a = sol[0];
            plane.b = sol[1];
            plane.c = 1.0;
            plane.d = sol[2];
        }
        note = std::string("d=1 неприменимо (плоскость через 0 или вырождение), нормировка ") + label;
        return true;
    };

    ok = try_fix(0, "a=1") || try_fix(1, "b=1") || try_fix(2, "c=1");
    if (!ok) {
        note = "три точки коллинеарны, плоскость не определяется";
    }
    return plane;
}

static Plane plane_method2(const Point3D& p1, const Point3D& p2, const Point3D& p3) {
    Point3D u{p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    Point3D v{p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
    Plane plane;
    plane.a = u.y * v.z - u.z * v.y;
    plane.b = u.z * v.x - u.x * v.z;
    plane.c = u.x * v.y - u.y * v.x;
    plane.d = -(plane.a * p1.x + plane.b * p1.y + plane.c * p1.z);
    return plane;
}

static Plane plane_method3(const Mesh& mesh, const Face& face) {
    Plane plane{0.0, 0.0, 0.0, 0.0};
    int n = static_cast<int>(face.vertex_indices.size());
    if (n < 3) {
        return plane;
    }
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        const Point3D& pi = mesh.vertices[face.vertex_indices[i]];
        const Point3D& pj = mesh.vertices[face.vertex_indices[j]];
        plane.a += (pi.y - pj.y) * (pi.z + pj.z);
        plane.b += (pi.z - pj.z) * (pi.x + pj.x);
        plane.c += (pi.x - pj.x) * (pi.y + pj.y);
    }
    const Point3D& p0 = mesh.vertices[face.vertex_indices[0]];
    plane.d = -(plane.a * p0.x + plane.b * p0.y + plane.c * p0.z);
    return plane;
}

PlaneTriple compute_face_three_ways(const Mesh& mesh, const Face& face) {
    PlaneTriple t;
    t.method1_ok = false;
    if (face.vertex_indices.size() < 3) {
        t.method1_note = "меньше трёх вершин";
        return t;
    }
    const Point3D& p1 = mesh.vertices[face.vertex_indices[0]];
    const Point3D& p2 = mesh.vertices[face.vertex_indices[1]];
    const Point3D& p3 = mesh.vertices[face.vertex_indices[2]];
    t.method1 = plane_method1(p1, p2, p3, t.method1_ok, t.method1_note);
    t.method2 = plane_method2(p1, p2, p3);
    t.method3 = plane_method3(mesh, face);
    return t;
}

std::vector<Plane> build_body_matrix(const Mesh& mesh) {
    Point3D interior = mesh_interior_point(mesh);
    std::vector<Plane> columns;
    for (const auto& face : mesh.faces) {
        Plane p = plane_method3(mesh, face);
        fix_plane_sign(p, interior);
        columns.push_back(p);
    }
    return columns;
}

bool is_back_face(const Plane& plane, const Point3D& eye) {
    return homogeneous_dot(plane, eye.x, eye.y, eye.z, 1.0) > 0.0;
}

static Point3D vec_sub(const Point3D& a, const Point3D& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

static Point3D vec_cross(const Point3D& a, const Point3D& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

static double vec_dot(const Point3D& a, const Point3D& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Point3D vec_norm(Point3D v) {
    double L = std::sqrt(vec_dot(v, v));
    if (L < 1e-12) {
        return {0.0, 0.0, 1.0};
    }
    return {v.x / L, v.y / L, v.z / L};
}

// Одна камера: глаз = точка наблюдения, смотрит на look_at. Сетка и тело в одном пространстве.
ScreenPt project_solid(double x, double y, double z, const ViewParams& view) {
    Point3D pos{x, y, z};
    Point3D forward = vec_norm(vec_sub(view.look_at, view.eye));
    Point3D world_up{0.0, 1.0, 0.0};
    Point3D right = vec_cross(forward, world_up);
    if (std::sqrt(vec_dot(right, right)) < 1e-8) {
        world_up = {0.0, 0.0, 1.0};
        right = vec_cross(forward, world_up);
    }
    right = vec_norm(right);
    Point3D up = vec_cross(right, forward);

    Point3D rel = vec_sub(pos, view.eye);
    double xc = vec_dot(rel, right);
    double yc = vec_dot(rel, up);
    double zc = vec_dot(rel, forward);
    if (zc < 1.0) {
        zc = 1.0;
    }

    double dist = std::sqrt(vec_dot(vec_sub(view.look_at, view.eye), vec_sub(view.look_at, view.eye)));
    double focal = 1.15 * dist * view.scene_scale;
    ScreenPt p;
    p.x = static_cast<int>(std::round(view.width * 0.5 + focal * xc / zc));
    p.y = static_cast<int>(std::round(view.height * 0.52 - focal * yc / zc));
    p.ok = (p.x >= 0 && p.x < view.width && p.y >= 0 && p.y < view.height);
    return p;
}

void draw_axes_grid_solid(Image& img, const ViewParams& view) {
    Pixel grid{190, 190, 190};
    Pixel xcol{200, 40, 40};
    Pixel ycol{40, 160, 40};
    Pixel zcol{40, 80, 200};
    const double extent = 140.0;
    const double step = 20.0;
    for (double t = -extent; t <= extent + 0.1; t += step) {
        ScreenPt a = project_solid(t, 0.0, -extent, view);
        ScreenPt b = project_solid(t, 0.0,  extent, view);
        ScreenPt c = project_solid(-extent, 0.0, t, view);
        ScreenPt d = project_solid( extent, 0.0, t, view);
        draw_line(img, a.x, a.y, b.x, b.y, grid);
        draw_line(img, c.x, c.y, d.x, d.y, grid);
    }
    ScreenPt o = project_solid(0, 0, 0, view);
    ScreenPt px = project_solid(extent, 0, 0, view);
    ScreenPt py = project_solid(0, extent, 0, view);
    ScreenPt pz = project_solid(0, 0, extent, view);
    draw_line(img, o.x, o.y, px.x, px.y, xcol);
    draw_line(img, o.x, o.y, py.x, py.y, ycol);
    draw_line(img, o.x, o.y, pz.x, pz.y, zcol);
}

static void draw_edge_pattern(Image& img, int x0, int y0, int x1, int y1,
                              const Pixel& color, bool dashed) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int step = 0;
    while (true) {
        bool plot = !dashed || ((step / 4) % 2 == 0);
        if (plot && x0 >= 0 && x0 < img.width && y0 >= 0 && y0 < img.height) {
            img.pixels[y0 * img.width + x0] = color;
        }
        if (x0 == x1 && y0 == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
        ++step;
    }
}

void draw_mesh_roberts(Image& img, const Mesh& mesh, const std::vector<bool>& back_face,
                       const ViewParams& view) {
    std::vector<ScreenPt> proj(mesh.vertices.size());
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        const Point3D& v = mesh.vertices[i];
        proj[i] = project_solid(v.x, v.y, v.z, view);
    }

    Pixel hidden{170, 170, 170};
    Pixel visible{20, 50, 170};

    auto key = [](int a, int b) {
        if (a > b) {
            std::swap(a, b);
        }
        return std::pair<int, int>(a, b);
    };

    auto draw_pass = [&](bool want_back) {
        std::set<std::pair<int, int>> drawn;
        for (size_t f = 0; f < mesh.faces.size(); ++f) {
            if (back_face[f] != want_back) {
                continue;
            }
            const Face& face = mesh.faces[f];
            int k = static_cast<int>(face.vertex_indices.size());
            for (int i = 0; i < k; ++i) {
                int a = face.vertex_indices[i];
                int b = face.vertex_indices[(i + 1) % k];
                auto e = key(a, b);
                if (drawn.count(e)) {
                    continue;
                }
                drawn.insert(e);
                draw_edge_pattern(img, proj[a].x, proj[a].y, proj[b].x, proj[b].y,
                                  want_back ? hidden : visible, want_back);
            }
        }
    };

    draw_pass(true);
    draw_pass(false);
}
