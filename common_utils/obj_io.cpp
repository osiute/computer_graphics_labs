#include "obj_io.h"
#include <fstream>
#include <iomanip>

bool write_wavefront_obj(const std::string& filename,
                         const std::string& object_name,
                         const std::vector<ObjVert>& vertices,
                         const std::vector<ObjFace>& faces) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        return false;
    }

    out << "# Wavefront OBJ, lab 15\n";
    out << "# vertices " << vertices.size() << ", faces " << faces.size() << "\n";
    out << "o " << object_name << "\n";
    out << std::fixed << std::setprecision(6);

    for (const auto& v : vertices) {
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }
    for (const auto& f : faces) {
        if (f.idx.size() < 3) {
            continue;
        }
        out << "f";
        for (int i : f.idx) {
            out << " " << (i + 1);
        }
        out << "\n";
    }
    return true;
}
