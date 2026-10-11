#pragma once
#include "image.h"
#include <string>

Image read_ppm_p3(const std::string& filename);
void write_ppm_p3(const std::string& filename, const Image& img);