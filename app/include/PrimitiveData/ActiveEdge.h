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
    float dx{};

    ActiveEdge(const QVector3D &upper, const QVector3D &lower) {
        yMax = static_cast<int>(std::floor(upper.y()));
        x = lower.x();

        const float dy = upper.y() - lower.y();
        if (std::abs(dy) > std::numeric_limits<float>::epsilon()) {
            dx = (upper.x() - lower.x()) / dy;
        } else {
            dx = 0.0f;
        }
    }
};

#endif //ACTIVEEDGE_H
