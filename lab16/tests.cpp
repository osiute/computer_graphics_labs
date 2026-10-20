#include "obj_mtl.h"
#include "png_io.h"
#include "renderer.h"
#include "triangulation.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

namespace {
namespace fs = std::filesystem;
using namespace lab16;

void check(bool value, const std::string& message) {
    if (!value) throw std::runtime_error(message);
}

template<class F>
void must_throw(F action) {
    bool thrown = false;
    try { action(); } catch (const std::exception&) { thrown = true; }
    check(thrown, "Expected an exception");
}

bool same(Pixel a, Pixel b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

int packed(Pixel p) { return (int(p.r) << 16) | (int(p.g) << 8) | int(p.b); }

std::size_t color_count(const Image& image, Pixel color) {
    return static_cast<std::size_t>(std::count_if(image.pixels.begin(), image.pixels.end(),
        [color](Pixel p) { return same(p, color); }));
}

bool same_image(const Image& a, const Image& b) {
    return a.width == b.width && a.height == b.height && a.pixels.size() == b.pixels.size()
        && std::equal(a.pixels.begin(), a.pixels.end(), b.pixels.begin(), same);
}

struct TempFiles {
    fs::path dir;
    TempFiles() {
        const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        dir = fs::temp_directory_path() / ("lab16-tests-" + std::to_string(stamp));
        check(fs::create_directory(dir), "Cannot create temporary test directory");
    }
    ~TempFiles() {
        std::error_code ec;
        fs::remove_all(dir, ec);
    }
    fs::path write(const std::string& name, const std::string& content) const {
        const fs::path path = dir / fs::u8path(name);
        fs::create_directories(path.parent_path());
        std::ofstream out(path, std::ios::binary);
        out << content;
        out.close();
        check(bool(out), "Cannot create fixture");
        return path;
    }
};

const std::string vertices = "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\n";

void test_obj_indices() {
    TempFiles temp;
    temp.write("colors.mtl", "newmtl Red\nKd 1 0 0\nnewmtl Blue\nKd 0 0 1\n");
    const auto path = temp.write("indices.obj", "\xef\xbb\xbfmtllib colors.mtl\n" + vertices
        + "vt 0 0\nvt 1 0\nvt 1 1\nvt 0 1\nvn 0 0 1\n"
          "usemtl Red\nf 1/1/1 2/2/1 3/3/1 4/4/1 # quad\n"
          "f -4/-4/-1 -3/-3/-1 -2/-2/-1\n"
          "usemtl Blue\nf 1//1 3//1 4//1\nf 1/1 2/2 4/4\nf 1 2 3\n");
    const auto model = load_obj(path);
    check(model.mesh.vertices.size() == 4 && model.mesh.faces.size() == 5, "OBJ counts");
    check(model.mesh.faces[0].vertex_indices == std::vector<int>({0, 1, 2, 3}), "Positive indices");
    check(model.mesh.faces[1].vertex_indices == std::vector<int>({0, 1, 2}), "Relative indices");
    check(same(face_color(model, 0), Pixel{255, 0, 0})
        && same(face_color(model, 1), Pixel{255, 0, 0})
        && same(face_color(model, 2), Pixel{0, 0, 255}), "Persistent usemtl");
    check(model.warnings.empty(), "Valid OBJ should not warn");

    temp.write("sub folder/палитра.mtl", "newmtl With spaces\nKd 0.2 0.4 0.6\n");
    const auto spaced = temp.write("space.obj", "mtllib \"sub folder/палитра.mtl\"\n" + vertices
        + "usemtl With spaces\nf 1 2 3\n");
    check(same(face_color(load_obj(spaced), 0), Pixel{51, 102, 153}), "Unicode and quoted relative MTL");
    const auto unquoted = temp.write("unquoted.obj", "mtllib sub folder/палитра.mtl\n" + vertices
        + "usemtl With spaces\nf 1 2 3\n");
    check(same(face_color(load_obj(unquoted), 0), Pixel{51, 102, 153}), "Unquoted existing MTL path");
    temp.write("second.mtl", "newmtl Green\nKd 0 1 0\n");
    const auto multiple = temp.write("multiple.obj", "mtllib colors.mtl second.mtl\n" + vertices
        + "usemtl Green\nf 1 2 3\n");
    check(same(face_color(load_obj(multiple), 0), Pixel{0, 255, 0}), "Multiple MTL libraries");
}

void test_bad_input_and_fallback() {
    TempFiles temp;
    must_throw([&] { load_obj(temp.dir / "missing.obj"); });
    must_throw([&] { load_obj(temp.write("empty.obj", "# empty\n")); });
    const std::vector<std::string> invalid = {
        "f 0 2 3\n", "f 1 2 9\n", "f -5 2 3\n", "f 1 2\n", "f 1 1 2\n",
        "f 1abc 2 3\n", "f 1/2 2/1 3/1\n", "f 1//1 2//1 3//1\n",
        "f 1///1 2 3\n", "f 1/ 2 3\n", "f 1// 2 3\n",
        "f 99999999999999999999999999999 2 3\n"
    };
    for (const auto& face : invalid) {
        must_throw([&] { load_obj(temp.write("invalid.obj", vertices + face)); });
    }
    for (const auto& coordinate : {"v nan 0 0\n", "v 1e999 0 0\n", "v 0 0\n", "v 0 0 0 0\n"}) {
        must_throw([&] { load_obj(temp.write("invalid.obj", coordinate + vertices + "f 1 2 3\n")); });
    }
    auto model = load_obj(temp.write("plain.obj", vertices + "f 1 2 3\n"));
    check(!model.warnings.empty() && same(face_color(model, 0), default_color), "No materials fallback");
    model = load_obj(temp.write("missing_mtl.obj", "mtllib absent.mtl\n" + vertices
        + "usemtl Missing\nf 1 2 3\n"));
    check(!model.warnings.empty() && same(face_color(model, 0), default_color), "Missing MTL fallback");
    temp.write("bad.mtl", "newmtl Broken\nKd 2 0 0\nnewmtl Valid\nKd 0 1 0\n");
    model = load_obj(temp.write("bad_mtl.obj", "mtllib bad.mtl\n" + vertices
        + "usemtl Broken\nf 1 2 3\nusemtl Valid\nf 1 3 4\n"));
    check(!model.warnings.empty() && same(face_color(model, 0), default_color)
        && same(face_color(model, 1), Pixel{0, 255, 0}), "Invalid Kd fallback keeps other material");
    for (const auto& kd : {"nan 0 0", "0 -1 0", "0 0", "0 0 0 1"}) {
        temp.write("bad.mtl", std::string("newmtl Broken\nKd ") + kd + "\n");
        model = load_obj(temp.dir / "bad_mtl.obj");
        check(!model.warnings.empty() && same(face_color(model, 0), default_color), "Bad Kd validation");
    }
}

void test_triangulation() {
    // U-shaped polygon: a fan from vertex 0 would fill part of the cutout.
    Mesh mesh;
    mesh.vertices = {{-3, -3, 0}, {3, -3, 0}, {3, 3, 0}, {1, 3, 0},
                     {1, -1, 0}, {-1, -1, 0}, {-1, 3, 0}, {-3, 3, 0}};
    Face face{{0, 1, 2, 3, 4, 5, 6, 7}};
    for (int winding = 0; winding < 2; ++winding) {
        const auto triangles = triangulate_face(mesh, face);
        check(triangles.size() == 6, "Concave n-gon triangle count");
        double area = 0;
        for (const auto& t : triangles) {
            const auto a = mesh.vertices[t[0]], b = mesh.vertices[t[1]], c = mesh.vertices[t[2]];
            area += std::abs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) / 2;
        }
        check(std::abs(area - 28) < 1e-10, "Concave polygon area");
        Model model;
        model.mesh = mesh;
        model.mesh.faces.push_back(face);
        RenderOptions options;
        options.width = options.height = 200;
        options.azimuth = options.elevation = 0;
        const auto rendered = render_model(model, options);
        check(same(rendered.image.pixels[80 * 200 + 100], options.background), "Concave cutout stays empty");
        check(same(rendered.image.pixels[80 * 200 + 50], default_color), "Concave lobe is filled");
        std::reverse(face.vertex_indices.begin(), face.vertex_indices.end());
    }
    mesh.vertices = {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {2, 2, 0}, {0, 2, 0}};
    check(triangulate_face(mesh, Face{{0, 1, 1, 2, 3, 4, 0}}).size() == 2,
          "Collinear/repeated/closing vertices");
    must_throw([&] { triangulate_face(mesh, Face{{0, 1, 2}}); });
    must_throw([&] { triangulate_face(mesh, Face{{0, 3, 2, 4}}); }); // bow-tie
    must_throw([&] { triangulate_face(mesh, Face{{0, 1, 99}}); });
    mesh.vertices = {{0, 0, 0}, {2, 0, 0}, {2, 2, 0.1}, {0, 2, 0}};
    check(triangulate_face(mesh, Face{{0, 1, 2, 3}}).size() == 2, "Nonplanar projected quad");
    for (double scale : {1e-100, 1e100}) {
        Mesh scaled = mesh;
        for (auto& p : scaled.vertices) { p.x *= scale; p.y *= scale; p.z *= scale; }
        check(triangulate_face(scaled, Face{{0, 1, 2, 3}}).size() == 2, "Scale-independent triangulation");
    }
}

Point3D subtract(Point3D a, Point3D b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
Point3D cross(Point3D a, Point3D b) {
    return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
double dot(Point3D a, Point3D b) { return a.x*b.x+a.y*b.y+a.z*b.z; }

// Independent visibility reference: intersect camera rays with 3D triangles.
double ray_hit(Point3D origin, Point3D direction, Point3D a, Point3D b, Point3D c) {
    const auto e1 = subtract(b, a), e2 = subtract(c, a);
    const auto h = cross(direction, e2);
    const double det = dot(e1, h);
    if (std::abs(det) < 1e-12) return std::numeric_limits<double>::infinity();
    const auto s = subtract(origin, a);
    const double u = dot(s, h) / det;
    const auto q = cross(s, e1);
    const double v = dot(direction, q) / det;
    const double distance = dot(e2, q) / det;
    if (u < 0 || v < 0 || u + v > 1 || distance <= 0) return std::numeric_limits<double>::infinity();
    return distance;
}

void test_depth_against_rays() {
    Model model;
    // A sloped triangle intersects a constant-Z triangle. This asymmetric
    // fixture exposes linear-Z interpolation errors after perspective division.
    model.mesh.vertices = {{-3, -2, 2}, {3, -2, -2}, {0, 3, 0},
                           {-3, -2, 0.35}, {3, -2, 0.35}, {0, 3, 0.35}};
    model.mesh.faces = {Face{{0, 1, 2}}, Face{{3, 4, 5}}};
    model.face_materials = {"Red", "Blue"};
    model.materials["Red"].diffuse = {255, 0, 0};
    model.materials["Blue"].diffuse = {0, 0, 255};
    RenderOptions options;
    options.width = 180;
    options.height = 160;
    options.azimuth = options.elevation = 0;
    const auto image = render_model(model, options).image;
    check(color_count(image, {255, 0, 0}) > 100 && color_count(image, {0, 0, 255}) > 100,
          "Intersecting surfaces both visible");
    // Independent bounding box normalization for this fixture.
    double r = 0;
    for (auto& p : model.mesh.vertices) { p.y -= 0.5; r = std::max(r, std::hypot(p.x, p.y, p.z)); }
    for (auto& p : model.mesh.vertices) { p.x *= 100/r; p.y *= 100/r; p.z *= 100/r; }
    const double focal = 0.43 * std::min(options.width, options.height) * std::sqrt(500.0*500-100.0*100) / 100;
    std::size_t compared = 0;
    for (int y = 0; y < options.height; ++y) {
        for (int x = 0; x < options.width; ++x) {
            const Point3D ray{(x+0.5-options.width/2.0)/focal,
                              -(y+0.5-options.height/2.0)/focal, -1};
            double distance = std::numeric_limits<double>::infinity();
            Pixel expected = options.background;
            for (std::size_t f = 0; f < 2; ++f) {
                const auto& indices = model.mesh.faces[f].vertex_indices;
                const double t = ray_hit({0, 0, 500}, ray, model.mesh.vertices[indices[0]],
                                         model.mesh.vertices[indices[1]], model.mesh.vertices[indices[2]]);
                if (t < distance) { distance = t; expected = face_color(model, f); }
            }
            check(same(image.pixels[static_cast<std::size_t>(y)*options.width+x], expected),
                  "Z-buffer differs from independent ray/triangle reference at "
                  + std::to_string(x) + "," + std::to_string(y));
            ++compared;
        }
    }
    check(compared == 28800, "Reference pixels count");
    // Restore coordinates and reverse submission order.
    for (auto& p : model.mesh.vertices) { p.x *= r/100; p.y = p.y*r/100+0.5; p.z *= r/100; }
    std::reverse(model.mesh.faces.begin(), model.mesh.faces.end());
    std::reverse(model.face_materials.begin(), model.face_materials.end());
    check(same_image(image, render_model(model, options).image), "Face order must not affect visibility");
}

void test_material_edit_png_and_cameras() {
    TempFiles temp;
    const fs::path assets = fs::u8path(LAB16_SOURCE_DIR) / "assets";
    fs::copy_file(assets / "house.obj", temp.dir / "house.obj");
    fs::copy_file(assets / "house.mtl", temp.dir / "house.mtl");
    RenderOptions options;
    options.width = 320;
    options.height = 240;
    auto model = load_obj(temp.dir / "house.obj");
    check(model.mesh.vertices.size() == 10 && model.mesh.faces.size() == 7, "Lab15 house geometry");
    const auto first = render_model(model, options);
    std::set<int> colors;
    for (Pixel p : first.image.pixels) if (!same(p, options.background)) colors.insert(packed(p));
    check(colors.size() >= 2, "House must show at least two exact material colors");
    check(first.triangles == 16, "House triangulation count");

    std::ifstream source(temp.dir / "house.mtl");
    std::ostringstream changed;
    std::string line;
    while (std::getline(source, line)) {
        std::istringstream fields(line);
        std::string instruction;
        fields >> instruction;
        changed << (instruction == "Kd" ? "Kd 0 1 0" : line) << '\n';
    }
    source.close();
    temp.write("house.mtl", changed.str());
    model = load_obj(temp.dir / "house.obj");
    const auto second = render_model(model, options);
    check(!same_image(first.image, second.image), "Editing Kd must change the rendered image");
    check(color_count(second.image, {0, 255, 0}) == second.visible_pixels,
          "Reloaded Kd must reach all rendered faces exactly");
    check(second.visible_pixels == first.visible_pixels, "Kd does not change geometry");

    const auto path = temp.dir / fs::u8path("результат.png");
    save_png(path, first.image);
    std::ifstream file(path, std::ios::binary);
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), {});
    int w = 0, h = 0, channels = 0;
    auto* pixels = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &w, &h, &channels, 3);
    check(pixels && w == options.width && h == options.height && channels == 3, "Decode saved PNG");
    bool identical = true;
    for (std::size_t i = 0; i < first.image.pixels.size(); ++i) {
        identical = identical && same(first.image.pixels[i], {pixels[3*i], pixels[3*i+1], pixels[3*i+2]});
    }
    stbi_image_free(pixels);
    check(identical, "PNG round trip preserves every RGB pixel");
    must_throw([&] { save_png(temp.dir / "missing" / "output.png", first.image); });
    for (double elevation : {-90.0, -25.0, 0.0, 25.0, 90.0}) {
        options.elevation = elevation;
        check(render_model(model, options).visible_pixels > 0, "Camera elevation including poles");
    }
    options.elevation = 91;
    must_throw([&] { render_model(model, options); });
    options.elevation = 0;
    options.width = 0;
    must_throw([&] { render_model(model, options); });
}

void test_previous_lab_models() {
    const fs::path root = fs::u8path(LAB16_SOURCE_DIR) / ".." / "lab15" / "obj";
    std::size_t tested = 0;
    RenderOptions options;
    options.width = options.height = 128;
    for (const auto& entry : fs::directory_iterator(root)) {
        if (entry.path().extension() != ".obj") continue;
        const auto result = render_model(load_obj(entry.path()), options);
        check(result.visible_pixels > 0, "No visible pixels in " + entry.path().u8string());
        std::cout << "  " << entry.path().filename().u8string() << ": " << result.triangles << " triangles\n";
        ++tested;
    }
    check(tested >= 10, "Expected all original Lab15 OBJ files");
}
} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"OBJ indices, materials and paths", test_obj_indices},
        {"Invalid data and missing material fallback", test_bad_input_and_fallback},
        {"Concave/degenerate/nonplanar triangulation", test_triangulation},
        {"Z-buffer vs 28800 independent ray intersections", test_depth_against_rays},
        {"Kd reload, PNG round trip and camera", test_material_edit_png_and_cameras},
        {"All Lab15 models", test_previous_lab_models}
    };
    int failed = 0;
    for (const auto& [name, run] : tests) {
        try { run(); std::cout << "PASS: " << name << '\n'; }
        catch (const std::exception& error) { ++failed; std::cerr << "FAIL: " << name << ": " << error.what() << '\n'; }
    }
    std::cout << tests.size() - static_cast<std::size_t>(failed) << "/" << tests.size() << " groups passed\n";
    return failed == 0 ? 0 : 1;
}
