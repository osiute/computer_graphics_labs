#pragma once

#include <cstddef>
#include "obj_mtl.h"

namespace lab16 {

struct RenderOptions {
    int width = 1000;
    int height = 800;
    double azimuth = 35.0;
    double elevation = 25.0;
    Pixel background{242, 245, 249};
};

struct RenderResult {
    Image image;
    std::size_t triangles = 0;
    std::size_t visible_pixels = 0;
};

// Камера смотрит в центр модели. Азимут 0: со стороны +Z, 90: со стороны +X.
RenderResult render_model(const Model& model, const RenderOptions& options = {});

} // namespace lab16
