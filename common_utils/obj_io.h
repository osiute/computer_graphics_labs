#pragma once
#include <string>
#include <vector>

struct ObjVert {
    double x, y, z;
};

struct ObjFace {
    std::vector<int> idx;
};

// Пишет Wavefront OBJ: комментарий, o имя, v, f (индексы с 1)
bool write_wavefront_obj(const std::string& filename,
                         const std::string& object_name,
                         const std::vector<ObjVert>& vertices,
                         const std::vector<ObjFace>& faces);
