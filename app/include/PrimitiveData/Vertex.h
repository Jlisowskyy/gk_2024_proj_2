//
// Created by Jlisowskyy on 05/11/24.
//

#ifndef APP_VERTEX_H
#define APP_VERTEX_H

/* external includes */
#include <QVector3D>

struct Vertex {
    /* before rotations */
    QVector3D position{};
    QVector3D puVector{};
    QVector3D pvVector{};
    QVector3D normal{};

    /* after rotations */
    QVector3D rotatedPosition{};
    QVector3D rotatedPuVector{};
    QVector3D rotatedPvVector{};
    QVector3D rotatedNormal{};

    /* base space coordinates */
    float u{};
    float v{};

    Vertex() = default;

    Vertex(const QVector3D &p, const QVector3D &pu, const QVector3D &pv, const QVector3D &n, float u, float v,
           double alpha, double beta);
};

#endif //APP_VERTEX_H
