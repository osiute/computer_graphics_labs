#include "parser.h"
#include <fstream>
#include <iostream>

bool load_scene_from_file(const std::string& filename, scene_data& data) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: Не удалось открыть файл " << filename << std::endl;
        return false;
    }

    if (!(file >> data.canvas_width >> data.canvas_height)) return false;

    int n = 0;
    if (!(file >> n)) return false;
    data.subject_polygon.resize(n);
    for (int i = 0; i < n; ++i) {
        file >> data.subject_polygon[i].x >> data.subject_polygon[i].y;
    }

    int m = 0;
    if (!(file >> m)) return false;
    data.clip_window.resize(m);
    for (int i = 0; i < m; ++i) {
        file >> data.clip_window[i].x >> data.clip_window[i].y;
    }

    file.close();
    return true;
}