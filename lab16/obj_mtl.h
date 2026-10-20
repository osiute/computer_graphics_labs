#pragma once

#include "geometry.h"

#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace lab16 {

inline constexpr Pixel default_color{180, 180, 190};

struct Material {
    Pixel diffuse = default_color;
};

struct Model {
    Mesh mesh;
    // One material name for every face; an empty name uses default_color.
    std::vector<std::string> face_materials;
    std::map<std::string, Material> materials;
    std::vector<std::string> warnings;
};

// Geometry errors throw std::runtime_error with the input path and line number.
// Missing/invalid materials produce warnings and retain the default color.
Model load_obj(const std::filesystem::path& path);
Pixel face_color(const Model& model, std::size_t face_index);

}  // namespace lab16
