#include "obj_mtl.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <limits>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace lab16 {
namespace {

std::string trim(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    return value.substr(begin, value.find_last_not_of(" \t\r\n") - begin + 1);
}

std::string strip_comment(const std::string& line) {
    char quote = 0;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quote != 0) {
            if (c == quote) {
                quote = 0;
            }
        } else if (c == '\'' || c == '"') {
            quote = c;
        } else if (c == '#') {
            return line.substr(0, i);
        }
    }
    return line;
}

std::vector<std::string> words(const std::string& value) {
    std::istringstream input(value);
    input.imbue(std::locale::classic());
    std::vector<std::string> result;
    std::string token;
    while (input >> token) {
        result.push_back(token);
    }
    return result;
}

std::string location(const std::filesystem::path& path, std::size_t line) {
    return path.u8string() + ":" + std::to_string(line) + ": ";
}

[[noreturn]] void fail(const std::filesystem::path& path, std::size_t line,
                       const std::string& message) {
    throw std::runtime_error(location(path, line) + message);
}

// OBJ/MTL permit a backslash to continue a record on the following line.
class Lines {
public:
    explicit Lines(const std::filesystem::path& path) : stream_(path) {}

    bool opened() const { return stream_.is_open(); }
    bool failed() const { return stream_.bad(); }
    std::size_t line() const { return first_line_; }

    bool next(std::string& result) {
        result.clear();
        std::string physical;
        bool any = false;
        while (std::getline(stream_, physical)) {
            ++physical_line_;
            if (!any) {
                first_line_ = physical_line_;
                any = true;
            }
            if (physical_line_ == 1 && physical.compare(0, 3, "\xEF\xBB\xBF") == 0) {
                physical.erase(0, 3);
            }
            physical = trim(strip_comment(physical));
            const bool continued = !physical.empty() && physical.back() == '\\';
            if (continued) {
                physical.pop_back();
            }
            result += physical;
            if (!continued) {
                return true;
            }
            result += ' ';
        }
        return any;
    }

private:
    std::ifstream stream_;
    std::size_t physical_line_ = 0;
    std::size_t first_line_ = 0;
};

std::pair<std::string, std::string> record(const std::string& line) {
    const auto end = line.find_first_of(" \t");
    if (end == std::string::npos) {
        return {line, {}};
    }
    return {line.substr(0, end), trim(line.substr(end + 1))};
}

bool number(const std::string& token, double& result) {
    std::istringstream input(token);
    input.imbue(std::locale::classic());
    input >> result;
    return !input.fail() && input.peek() == std::char_traits<char>::eof() &&
           std::isfinite(result);
}

std::vector<double> coordinates(const std::string& value, std::size_t min_count,
                                std::size_t max_count,
                                const std::filesystem::path& path,
                                std::size_t line, const std::string& directive) {
    const auto tokens = words(value);
    if (tokens.size() < min_count || tokens.size() > max_count) {
        fail(path, line, directive + ": неверное количество координат");
    }
    std::vector<double> result;
    for (const auto& token : tokens) {
        double n = 0;
        if (!number(token, n)) {
            fail(path, line, directive + ": некорректное конечное число '" + token + "'");
        }
        result.push_back(n);
    }
    return result;
}

int index(const std::string& token, std::size_t count,
          const std::filesystem::path& path, std::size_t line,
          const std::string& kind) {
    long long raw = 0;
    const char* begin = token.data();
    const char* end = begin + token.size();
    if (begin != end && *begin == '+') {
        ++begin;
        if (begin == end || *begin == '-' || *begin == '+') {
            fail(path, line, "f: некорректный индекс " + kind + " '" + token + "'");
        }
    }
    const auto parsed = std::from_chars(begin, end, raw);
    if (parsed.ec != std::errc{} || parsed.ptr != end || raw == 0) {
        fail(path, line, "f: некорректный индекс " + kind + " '" + token + "'");
    }
    // Avoid signed arithmetic on LLONG_MIN and size_t underflow.
    if ((raw > 0 && static_cast<unsigned long long>(raw) > count) ||
        (raw < 0 && raw < -static_cast<long long>(count))) {
        fail(path, line, "f: индекс " + kind + " '" + token +
                             "' вне диапазона (объявлено " + std::to_string(count) + ")");
    }
    const auto resolved = raw > 0 ? raw - 1 : static_cast<long long>(count) + raw;
    if (resolved > std::numeric_limits<int>::max()) {
        fail(path, line, "f: индекс не помещается в тип индекса Mesh");
    }
    return static_cast<int>(resolved);
}

int face_vertex(const std::string& token, std::size_t vertices, std::size_t textures,
                std::size_t normals, const std::filesystem::path& path,
                std::size_t line) {
    const auto first = token.find('/');
    if (first == std::string::npos) {
        return index(token, vertices, path, line, "v");
    }
    const int v = index(token.substr(0, first), vertices, path, line, "v");
    const auto second = token.find('/', first + 1);
    if (second == std::string::npos) {
        index(token.substr(first + 1), textures, path, line, "vt");
        return v;
    }
    if (token.find('/', second + 1) != std::string::npos) {
        fail(path, line, "f: слишком много разделителей '/' в '" + token + "'");
    }
    if (second != first + 1) {
        index(token.substr(first + 1, second - first - 1), textures, path, line, "vt");
    }
    index(token.substr(second + 1), normals, path, line, "vn");
    return v;
}

// Names can contain spaces. Quoting an entire name is accepted as a convenience.
std::string material_name(const std::string& value) {
    const auto cleaned = trim(value);
    if (cleaned.size() >= 2 &&
        ((cleaned.front() == '"' && cleaned.back() == '"') ||
         (cleaned.front() == '\'' && cleaned.back() == '\''))) {
        return cleaned.substr(1, cleaned.size() - 2);
    }
    return cleaned;
}

std::vector<std::string> library_names(const std::string& value,
                                     const std::filesystem::path& directory) {
    std::error_code ec;
    // Some exporters emit one unquoted filename containing spaces.
    if (!value.empty() &&
        std::filesystem::is_regular_file(directory / std::filesystem::u8path(value), ec)) {
        return {value};
    }
    std::vector<std::string> result;
    std::string current;
    char quote = 0;
    for (const char c : value) {
        if (quote != 0) {
            if (c == quote) {
                quote = 0;
            } else {
                current += c;
            }
        } else if (c == '\'' || c == '"') {
            quote = c;
        } else if (c == ' ' || c == '\t') {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (quote != 0) {
        throw std::runtime_error("mtllib: незакрытая кавычка в имени файла");
    }
    if (!current.empty()) {
        result.push_back(current);
    }
    return result;
}

void load_mtl(const std::filesystem::path& path, Model& model) {
    Lines input(path);
    if (!input.opened()) {
        model.warnings.push_back("Не удалось открыть MTL '" + path.u8string() +
                                 "'. Для отсутствующих материалов используется стандартный цвет.");
        return;
    }
    std::string current;
    std::string line;
    while (input.next(line)) {
        const auto [directive, value] = record(line);
        if (directive == "newmtl") {
            current = material_name(value);
            if (current.empty()) {
                model.warnings.push_back(location(path, input.line()) + "newmtl: пустое имя материала");
                continue;
            }
            if (model.materials.count(current) != 0) {
                model.warnings.push_back(location(path, input.line()) + "повторное определение материала '" +
                                         current + "'; используется последнее определение");
            }
            model.materials[current] = Material{};
        } else if (directive == "Kd") {
            if (current.empty()) {
                model.warnings.push_back(location(path, input.line()) + "Kd до newmtl; запись пропущена");
                continue;
            }
            const auto tokens = words(value);
            double rgb[3]{};
            bool valid = tokens.size() == 3;
            if (valid) {
                for (std::size_t i = 0; i < 3; ++i) {
                    valid = number(tokens[i], rgb[i]) && rgb[i] >= 0 && rgb[i] <= 1 && valid;
                }
            }
            if (!valid) {
                model.materials[current].diffuse = default_color;
                model.warnings.push_back(location(path, input.line()) + "Kd: ожидаются три числа от 0 до 1; "
                                         "для материала '" + current + "' используется стандартный цвет");
                continue;
            }
            model.materials[current].diffuse = {
                static_cast<unsigned char>(std::lround(rgb[0] * 255)),
                static_cast<unsigned char>(std::lround(rgb[1] * 255)),
                static_cast<unsigned char>(std::lround(rgb[2] * 255))};
        }
    }
    if (input.failed()) {
        model.warnings.push_back("Ошибка чтения MTL '" + path.u8string() + "'");
    }
}

}  // namespace

Model load_obj(const std::filesystem::path& path) {
    Lines input(path);
    if (!input.opened()) {
        throw std::runtime_error("Не удалось открыть OBJ '" + path.u8string() + "'");
    }
    Model model;
    std::size_t textures = 0;
    std::size_t normals = 0;
    std::string current_material;
    bool has_library = false;
    std::string line;
    while (input.next(line)) {
        const auto [directive, value] = record(line);
        if (directive == "v") {
            const auto xyz = coordinates(value, 3, 4, path, input.line(), "v");
            const double w = xyz.size() == 4 ? xyz[3] : 1.0;
            if (w == 0 || !std::isfinite(xyz[0] / w) || !std::isfinite(xyz[1] / w) ||
                !std::isfinite(xyz[2] / w)) {
                fail(path, input.line(), "v: недопустимая однородная координата w");
            }
            if (model.mesh.vertices.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max())) {
                fail(path, input.line(), "слишком много вершин для типа индекса Mesh");
            }
            model.mesh.vertices.push_back({xyz[0] / w, xyz[1] / w, xyz[2] / w});
        } else if (directive == "vt") {
            coordinates(value, 1, 3, path, input.line(), "vt");
            ++textures;
        } else if (directive == "vn") {
            coordinates(value, 3, 3, path, input.line(), "vn");
            ++normals;
        } else if (directive == "f") {
            Face face;
            for (const auto& token : words(value)) {
                face.vertex_indices.push_back(face_vertex(token, model.mesh.vertices.size(), textures,
                                                          normals, path, input.line()));
            }
            const std::set<int> distinct(face.vertex_indices.begin(), face.vertex_indices.end());
            if (distinct.size() < 3) {
                fail(path, input.line(), "f: грань должна содержать как минимум три различные вершины");
            }
            model.mesh.faces.push_back(std::move(face));
            model.face_materials.push_back(current_material);
        } else if (directive == "usemtl") {
            current_material = material_name(value);
            if (current_material.empty()) {
                fail(path, input.line(), "usemtl: пустое имя материала");
            }
        } else if (directive == "mtllib") {
            has_library = true;
            try {
                const auto libraries = library_names(value, path.parent_path());
                if (libraries.empty()) {
                    model.warnings.push_back(location(path, input.line()) + "mtllib: не указано имя файла");
                }
                for (const auto& library : libraries) {
                    load_mtl(path.parent_path() / std::filesystem::u8path(library), model);
                }
            } catch (const std::runtime_error& error) {
                model.warnings.push_back(location(path, input.line()) + error.what());
            }
        }
    }
    if (input.failed()) {
        throw std::runtime_error("Ошибка чтения OBJ '" + path.u8string() + "'");
    }
    if (model.mesh.vertices.empty() || model.mesh.faces.empty()) {
        throw std::runtime_error("OBJ '" + path.u8string() + "' должен содержать вершины v и грани f");
    }
    if (!has_library) {
        model.warnings.push_back("OBJ не содержит mtllib. Используется стандартный цвет для граней без материала.");
    }
    std::set<std::string> unknown;
    bool unassigned = false;
    for (const auto& name : model.face_materials) {
        if (name.empty()) {
            unassigned = true;
        } else if (model.materials.count(name) == 0) {
            unknown.insert(name);
        }
    }
    if (unassigned && has_library) {
        model.warnings.push_back("Часть граней не имеет usemtl; используется стандартный цвет.");
    }
    for (const auto& name : unknown) {
        model.warnings.push_back("Материал '" + name + "' не найден в MTL; используется стандартный цвет.");
    }
    return model;
}

Pixel face_color(const Model& model, std::size_t face_index) {
    if (face_index >= model.face_materials.size()) {
        return default_color;
    }
    const auto found = model.materials.find(model.face_materials[face_index]);
    return found == model.materials.end() ? default_color : found->second.diffuse;
}

}  // namespace lab16
