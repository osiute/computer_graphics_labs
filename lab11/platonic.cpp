#include "platonic.h"
#include <cmath>

// 1. Гексаэдр / Куб (8 вершин, 12 ребер)
WireframeModel create_cube() {
    WireframeModel model;
    double s = 120.0; // Полусторона куба
    
    model.vertices = {
        {-s, -s, -s}, {s, -s, -s}, {s, s, -s}, {-s, s, -s}, // Нижняя грань (0-3)
        {-s, -s,  s}, {s, -s,  s}, {s, s,  s}, {-s, s,  s}  // Верхняя грань (4-7)
    };
    
    model.edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Низ
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Верх
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Соединения
    };
    
    return model;
}

// 2. Додекаэдр (20 вершин, 30 ребер) на основе золотого сечения
WireframeModel create_dodecahedron() {
    WireframeModel model;
    double phi = (1.0 + std::sqrt(5.0)) / 2.0; // Золотое сечение (~1.618)
    double inv_phi = 1.0 / phi;
    double scale = 80.0;

    // 20 вершин додекаэдра
    model.vertices = {
        // 8 кубических вершин
        {-scale, -scale, -scale}, {scale, -scale, -scale},
        {scale, scale, -scale}, {-scale, scale, -scale},
        {-scale, -scale,  scale}, {scale, -scale,  scale},
        {scale,  scale,  scale}, {-scale,  scale,  scale},
        // 12 вершин на плоскостях осей
        {0.0, -scale * inv_phi, -scale * phi}, {0.0,  scale * inv_phi, -scale * phi},
        {0.0, -scale * inv_phi,  scale * phi}, {0.0,  scale * inv_phi,  scale * phi},
        {-scale * inv_phi, -scale * phi, 0.0}, {scale * inv_phi, -scale * phi, 0.0},
        {scale * inv_phi,  scale * phi, 0.0}, {-scale * inv_phi,  scale * phi, 0.0},
        {-scale * phi, 0.0, -scale * inv_phi}, {scale * phi, 0.0, -scale * inv_phi},
        {scale * phi, 0.0,  scale * inv_phi}, {-scale * phi, 0.0,  scale * inv_phi}
    };

    // 30 ребер додекаэдра
    model.edges = {
        {0, 8}, {0, 12}, {0, 16},
        {1, 8}, {1, 13}, {1, 17},
        {2, 9}, {2, 14}, {2, 17},
        {3, 9}, {3, 15}, {3, 16},
        {4, 10}, {4, 12}, {4, 19},
        {5, 10}, {5, 13}, {5, 18},
        {6, 11}, {6, 14}, {6, 18},
        {7, 11}, {7, 15}, {7, 19},
        {8, 9}, {10, 11}, {12, 13}, {14, 15}, {16, 19}, {17, 18}
    };

    return model;
}

// Центральная проекция строго по формулам из методички (стр. 8):
// x_p = (d * x) / z,  y_p = (d * y) / z
Point3D apply_perspective(const Point3D& p, double d, double center_x, double center_y) {
    // Удаление объекта по оси Z (камера расположена на расстоянии 400 единиц)
    double z_dist = p.z + 450.0; 
    if (z_dist < 1.0) z_dist = 1.0;

    // Формулы из методички
    double xp = (d * p.x) / z_dist;
    double yp = (d * p.y) / z_dist;

    // Переход к экранным координатам (инвертируем Y для экрана)
    return { center_x + xp, center_y - yp, z_dist };
}