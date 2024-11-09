//
// Created by Jlisowskyy on 11/6/24.
//

#ifndef ACTIVEEDGE_H
#define ACTIVEEDGE_H

/* External includes */
#include <QVector3D>
#include <cmath>

struct ActiveEdge {
    int yMax{};
    float x{};
    float z{};
    float dx{};
    float dz{};

    ActiveEdge(const QVector3D &upper, const QVector3D &lower) {
        yMax = static_cast<int>(std::floor(upper.y()));
        x = lower.x();
        z = lower.z();

        const float dy = upper.y() - lower.y();
        if (std::abs(dy) > std::numeric_limits<float>::epsilon()) {
            dx = (upper.x() - lower.x()) / dy;
            dz = (upper.z() - lower.z()) / dy;
        } else {
            dx = 0.0f;
            dz = 0.0f;
        }
    }

    void update() {
        x += dx;
        z += dz;
    }
};

#endif //ACTIVEEDGE_H
