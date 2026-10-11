#include "svg_handler.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>

void save_segments_to_svg(const std::string& filename, const std::vector<Segment>& segments, int width, int height) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось создать SVG файл: " << filename << std::endl;
        return;
    }

    // Записываем XML/SVG заголовок
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    
    // Белый фон для удобства просмотра
    file << "  <rect width=\"100%\" height=\"100%\" fill=\"white\" />\n";

    // Записываем каждый отрезок в виде тега <line>
    for (const auto& seg : segments) {
        file << "  <line x1=\"" << seg.p1.x << "\" y1=\"" << seg.p1.y 
             << "\" x2=\"" << seg.p2.x << "\" y2=\"" << seg.p2.y 
             << "\" stroke=\"black\" stroke-width=\"2\" />\n";
    }

    // Закрывающий тег SVG
    file << "</svg>\n";
    file.close();
}

std::vector<Segment> read_segments_from_svg(const std::string& filename) {
    std::vector<Segment> segments;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть SVG файл для чтения: " << filename << std::endl;
        return segments;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ищем строки, содержащие тег линии
        if (line.find("<line") != std::string::npos) {
            Segment seg;
            // С помощью sscanf извлекаем координаты из атрибутов x1, y1, x2, y2
            int matched = std::sscanf(line.c_str(), 
                " <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"", 
                &seg.p1.x, &seg.p1.y, &seg.p2.x, &seg.p2.y);
            
            // На случай, если отступы в строке другие, попробуем альтернативный формат с scanf
            if (matched != 4) {
                matched = std::sscanf(line.c_str(), 
                    "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"", 
                    &seg.p1.x, &seg.p1.y, &seg.p2.x, &seg.p2.y);
            }

            if (matched == 4) {
                segments.push_back(seg);
            }
        }
    }

    file.close();
    return segments;
}