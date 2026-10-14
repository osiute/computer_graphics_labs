#pragma once
#include "geometry.h"
#include <vector>
#include <string>

struct ViewParams {
    double scene_scale;
    double azimuth_deg;
    double elevation_deg;
    int width;
    int height;
    Point3D eye;
    Point3D look_at;
};

struct ScreenPt {
    int x;
    int y;
    bool ok;
};

struct HorizonState {
    std::vector<double> upper;
    std::vector<double> lower;
};

double surface_y(double x, double z);
ScreenPt project_point(double x, double y, double z, const ViewParams& view);
void draw_axes_grid(Image& img, const ViewParams& view);
void draw_cutting_planes(Image& img, const std::vector<double>& z_planes, const ViewParams& view);

// Сечёт поверхность плоскостями z=const от ближней к дальней.
// Верхний/нижний горизонты — массивы по пикселю X.
void floating_horizon(Image& img, const std::vector<double>& z_planes,
                      const ViewParams& view, HorizonState* last_horizons);

void draw_horizon_polylines(Image& img, const HorizonState& h, const Pixel& up_color, const Pixel& lo_color);

struct Plane {
    double a, b, c, d;
};

struct PlaneTriple {
    Plane method1;
    bool method1_ok;
    std::string method1_note;
    Plane method2;
    Plane method3;
};

Mesh create_parallelepiped(double hx, double hy, double hz);
Mesh create_triangular_prism(double radius, double height);
Mesh create_house();
Mesh load_mesh_from_stdin();

Point3D mesh_interior_point(const Mesh& mesh);
PlaneTriple compute_face_three_ways(const Mesh& mesh, const Face& face);
std::vector<Plane> build_body_matrix(const Mesh& mesh);
double homogeneous_dot(const Plane& plane, double x, double y, double z, double w);
bool is_back_face(const Plane& plane, const Point3D& eye);

ScreenPt project_solid(double x, double y, double z, const ViewParams& view);
void draw_axes_grid_solid(Image& img, const ViewParams& view);
void draw_mesh_roberts(Image& img, const Mesh& mesh, const std::vector<bool>& back_face,
                       const ViewParams& view);
