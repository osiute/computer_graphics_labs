#pragma once

#include <filesystem>
#include "image.h"

namespace lab16 {
void save_png(const std::filesystem::path& path, const Image& image);
}
