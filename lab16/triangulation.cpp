#include "triangulation.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace lab16 {
namespace {

// All geometric decisions are made after scaling the face to a unit box.
// Thus these tolerances do not depend on the units used by the OBJ file.
constexpr long double length_tolerance = 1.0e-10L;
constexpr long double area_tolerance = 1.0e-12L;

struct Vector3 {
    long double x;
    long double y;
    long double z;
};

struct Vertex {
    int index;
    Vector3 position;
    long double x = 0;
    long double y = 0;
};

[[noreturn]] void invalid_face(const char* reason) {
    throw std::runtime_error(std::string("Cannot triangulate face: ") + reason);
}

Vector3 subtract(const Vector3& a, const Vector3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3 cross(const Vector3& a, const Vector3& b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

long double squared_length(const Vector3& p) {
    return p.x * p.x + p.y * p.y + p.z * p.z;
}

bool same_position(const Vertex& a, const Vertex& b) {
    return squared_length(subtract(a.position, b.position)) <=
           length_tolerance * length_tolerance;
}

long double orient(const Vertex& a, const Vertex& b, const Vertex& c) {
    return (b.x - a.x) * (c.y - a.y) -
           (b.y - a.y) * (c.x - a.x);
}

long double projected_distance_squared(const Vertex& a, const Vertex& b) {
    const long double dx = a.x - b.x;
    const long double dy = a.y - b.y;
    return dx * dx + dy * dy;
}

bool on_segment(const Vertex& a, const Vertex& b, const Vertex& p) {
    return std::abs(orient(a, b, p)) <= area_tolerance &&
           p.x >= std::min(a.x, b.x) - length_tolerance &&
           p.x <= std::max(a.x, b.x) + length_tolerance &&
           p.y >= std::min(a.y, b.y) - length_tolerance &&
           p.y <= std::max(a.y, b.y) + length_tolerance;
}

bool opposite_sides(long double a, long double b) {
    return (a > area_tolerance && b < -area_tolerance) ||
           (a < -area_tolerance && b > area_tolerance);
}

bool segments_intersect(const Vertex& a, const Vertex& b,
                        const Vertex& c, const Vertex& d) {
    const long double abc = orient(a, b, c);
    const long double abd = orient(a, b, d);
    const long double cda = orient(c, d, a);
    const long double cdb = orient(c, d, b);
    if (opposite_sides(abc, abd) && opposite_sides(cda, cdb)) {
        return true;
    }
    return on_segment(a, b, c) || on_segment(a, b, d) ||
           on_segment(c, d, a) || on_segment(c, d, b);
}

void clean_projected_contour(std::vector<Vertex>& vertices) {
    bool changed = true;
    while (changed && vertices.size() >= 3) {
        changed = false;
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            const Vertex& previous =
                vertices[(i + vertices.size() - 1) % vertices.size()];
            const Vertex& current = vertices[i];
            const Vertex& next = vertices[(i + 1) % vertices.size()];
            if (projected_distance_squared(previous, current) <=
                length_tolerance * length_tolerance) {
                vertices.erase(vertices.begin() + static_cast<std::ptrdiff_t>(i));
                changed = true;
                break;
            }
            if (std::abs(orient(previous, current, next)) <= area_tolerance) {
                // A redundant point lies between its neighbours. A reversal
                // instead overlaps two edges and is an invalid contour.
                const long double dot =
                    (current.x - previous.x) * (current.x - next.x) +
                    (current.y - previous.y) * (current.y - next.y);
                if (dot > length_tolerance * length_tolerance) {
                    invalid_face("overlapping adjacent edges in projection");
                }
                vertices.erase(vertices.begin() + static_cast<std::ptrdiff_t>(i));
                changed = true;
                break;
            }
        }
    }
    if (vertices.size() < 3) {
        invalid_face("fewer than three distinct non-collinear vertices");
    }
}

void check_simple_contour(const std::vector<Vertex>& vertices) {
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const std::size_t next_i = (i + 1) % vertices.size();
        for (std::size_t j = i + 1; j < vertices.size(); ++j) {
            const std::size_t next_j = (j + 1) % vertices.size();
            if (next_i == j || next_j == i) {
                continue;  // Adjacent edges share exactly one endpoint.
            }
            if (segments_intersect(vertices[i], vertices[next_i],
                                   vertices[j], vertices[next_j])) {
                invalid_face("self-intersecting contour in projection");
            }
        }
    }
}

bool point_in_triangle(const Vertex& p, const Vertex& a, const Vertex& b,
                       const Vertex& c, long double winding) {
    // Include the boundary so a diagonal cannot pass through another vertex.
    return winding * orient(a, b, p) >= -area_tolerance &&
           winding * orient(b, c, p) >= -area_tolerance &&
           winding * orient(c, a, p) >= -area_tolerance;
}

}  // namespace

std::vector<Triangle> triangulate_face(const Mesh& mesh, const Face& face) {
    if (face.vertex_indices.size() < 3) {
        invalid_face("fewer than three vertices");
    }

    std::vector<Vertex> vertices;
    vertices.reserve(face.vertex_indices.size());
    Vector3 minimum{};
    Vector3 maximum{};
    bool first = true;
    for (int index : face.vertex_indices) {
        if (index < 0 || static_cast<std::size_t>(index) >= mesh.vertices.size()) {
            invalid_face("vertex index is out of range");
        }
        const Point3D& source = mesh.vertices[static_cast<std::size_t>(index)];
        if (!std::isfinite(source.x) || !std::isfinite(source.y) ||
            !std::isfinite(source.z)) {
            invalid_face("non-finite vertex coordinate");
        }
        const Vector3 p{source.x, source.y, source.z};
        vertices.push_back({index, p});
        if (first) {
            minimum = maximum = p;
            first = false;
        } else {
            minimum.x = std::min(minimum.x, p.x);
            minimum.y = std::min(minimum.y, p.y);
            minimum.z = std::min(minimum.z, p.z);
            maximum.x = std::max(maximum.x, p.x);
            maximum.y = std::max(maximum.y, p.y);
            maximum.z = std::max(maximum.z, p.z);
        }
    }

    const long double scale = std::max({maximum.x - minimum.x,
                                        maximum.y - minimum.y,
                                        maximum.z - minimum.z});
    if (!(scale > 0)) {
        invalid_face("all vertices coincide");
    }
    for (Vertex& vertex : vertices) {
        vertex.position = {(vertex.position.x - minimum.x) / scale,
                           (vertex.position.y - minimum.y) / scale,
                           (vertex.position.z - minimum.z) / scale};
    }

    // OBJ exporters sometimes append the initial vertex to close the face.
    std::vector<Vertex> unique;
    unique.reserve(vertices.size());
    for (const Vertex& vertex : vertices) {
        if (unique.empty() || !same_position(unique.back(), vertex)) {
            unique.push_back(vertex);
        }
    }
    if (unique.size() > 1 && same_position(unique.front(), unique.back())) {
        unique.pop_back();
    }
    vertices = std::move(unique);
    if (vertices.size() < 3) {
        invalid_face("fewer than three distinct vertices");
    }

    Vector3 normal{0, 0, 0};
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const Vector3& a = vertices[i].position;
        const Vector3& b = vertices[(i + 1) % vertices.size()].position;
        const Vector3 contribution = cross(a, b);
        normal.x += contribution.x;
        normal.y += contribution.y;
        normal.z += contribution.z;
    }
    if (squared_length(normal) <= area_tolerance * area_tolerance) {
        // A self-intersecting bow-tie can cancel the Newell normal. Keep a
        // nonzero local normal so the intersection check can explain it.
        for (std::size_t i = 1; i + 1 < vertices.size(); ++i) {
            const Vector3 candidate =
                cross(subtract(vertices[i].position, vertices[0].position),
                      subtract(vertices[i + 1].position, vertices[0].position));
            if (squared_length(candidate) > squared_length(normal)) {
                normal = candidate;
            }
        }
    }
    if (squared_length(normal) <= area_tolerance * area_tolerance) {
        invalid_face("zero area");
    }

    const long double nx = std::abs(normal.x);
    const long double ny = std::abs(normal.y);
    const long double nz = std::abs(normal.z);
    for (Vertex& vertex : vertices) {
        if (nx >= ny && nx >= nz) {
            vertex.x = vertex.position.y;
            vertex.y = vertex.position.z;
        } else if (ny >= nz) {
            vertex.x = vertex.position.x;
            vertex.y = vertex.position.z;
        } else {
            vertex.x = vertex.position.x;
            vertex.y = vertex.position.y;
        }
    }

    clean_projected_contour(vertices);
    check_simple_contour(vertices);
    long double twice_area = 0;
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const Vertex& a = vertices[i];
        const Vertex& b = vertices[(i + 1) % vertices.size()];
        twice_area += a.x * b.y - a.y * b.x;
    }
    if (std::abs(twice_area) <= area_tolerance) {
        invalid_face("zero projected area");
    }
    const long double winding = twice_area > 0 ? 1 : -1;

    std::vector<std::size_t> active(vertices.size());
    std::iota(active.begin(), active.end(), std::size_t{0});
    std::vector<Triangle> triangles;
    triangles.reserve(vertices.size() - 2);
    while (active.size() > 3) {
        bool found_ear = false;
        for (std::size_t i = 0; i < active.size(); ++i) {
            const std::size_t previous = active[(i + active.size() - 1) % active.size()];
            const std::size_t current = active[i];
            const std::size_t next = active[(i + 1) % active.size()];
            if (winding * orient(vertices[previous], vertices[current], vertices[next]) <=
                area_tolerance) {
                continue;
            }
            bool contains_vertex = false;
            for (std::size_t candidate : active) {
                if (candidate != previous && candidate != current && candidate != next &&
                    point_in_triangle(vertices[candidate], vertices[previous],
                                      vertices[current], vertices[next], winding)) {
                    contains_vertex = true;
                    break;
                }
            }
            if (contains_vertex) {
                continue;
            }
            triangles.push_back({vertices[previous].index, vertices[current].index,
                                 vertices[next].index});
            active.erase(active.begin() + static_cast<std::ptrdiff_t>(i));
            found_ear = true;
            break;
        }
        if (!found_ear) {
            invalid_face("numerically degenerate projected polygon");
        }
    }
    if (winding * orient(vertices[active[0]], vertices[active[1]], vertices[active[2]]) <=
        area_tolerance) {
        invalid_face("degenerate final triangle");
    }
    triangles.push_back({vertices[active[0]].index, vertices[active[1]].index,
                         vertices[active[2]].index});
    return triangles;
}

}  // namespace lab16
