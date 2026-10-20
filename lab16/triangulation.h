#pragma once

#include <array>
#include <vector>

#include "geometry.h"

namespace lab16 {

using Triangle = std::array<int, 3>;

// Returns triangles containing the original, zero-based mesh vertex indices.
// Handles concave polygons and either winding. Consecutive repeated vertices,
// a repeated closing vertex and redundant collinear vertices are discarded.
// A nonplanar face is defined by the triangles obtained after projection along
// the dominant component of its Newell normal. A degenerate or self-intersecting
// projected contour is rejected with std::runtime_error.
std::vector<Triangle> triangulate_face(const Mesh& mesh, const Face& face);

}  // namespace lab16
