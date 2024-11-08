//
// Created by Jlisowskyy on 11/8/24.
//

/* Main header */
#include "../include/Rendering/Mesh.h"

/* external includes */
#include <cmath>


Mesh::Mesh(QObject *parent, const ControlPoints &controlPoints, const float alpha, const float beta,
           const int accuracy): QObject(parent),
                                m_triangleAccuracy(accuracy),
                                m_alpha(alpha),
                                m_beta(beta),
                                m_controlPoints(controlPoints),
                                m_triangles(_interpolateBezier(controlPoints)) {
}

void Mesh::setAlpha(const double alpha) {
    m_alpha = static_cast<float>(alpha);
    m_triangles = _interpolateBezier(m_controlPoints);
}

void Mesh::setBeta(const double beta) {
    m_beta = static_cast<float>(beta);
    m_triangles = _interpolateBezier(m_controlPoints);
}

void Mesh::setAccuracy(const double accuracy) {
    m_triangleAccuracy = static_cast<int>(accuracy);
    m_triangles = _interpolateBezier(m_controlPoints);
}

MeshArr Mesh::_interpolateBezier(const ControlPoints &controlPoints) const {
    MeshArr arr{};

    const float step = 1.0f / static_cast<float>(m_triangleAccuracy - 1);
    const int steps = m_triangleAccuracy;

    for (int i = 0; i < steps - 1; ++i) {
        for (int j = 0; j < steps - 1; ++j) {
            const float u = static_cast<float>(i) * step;
            const float v = static_cast<float>(j) * step;
            const float u_next = static_cast<float>(i + 1) * step;
            const float v_next = static_cast<float>(j + 1) * step;

            const auto bu = _computeBernstein(u);
            const auto bv = _computeBernstein(v);
            const auto bu_next = _computeBernstein(u_next);
            const auto bv_next = _computeBernstein(v_next);

            const auto [p00, pu00, pv00] = _computePointAndDeriv(controlPoints, bu, bv);
            const auto [p10, pu10, pv10] = _computePointAndDeriv(controlPoints, bu_next, bv);
            const auto [p01, pu01, pv01] = _computePointAndDeriv(controlPoints, bu, bv_next);
            const auto [p11, pu11, pv11] = _computePointAndDeriv(controlPoints, bu_next, bv_next);

            const QVector3D n00 = QVector3D::crossProduct(pu00, pv00).normalized();
            const QVector3D n10 = QVector3D::crossProduct(pu10, pv10).normalized();
            const QVector3D n01 = QVector3D::crossProduct(pu01, pv01).normalized();
            const QVector3D n11 = QVector3D::crossProduct(pu11, pv11).normalized();

            Triangle t1, t2;

            t1[0] = Vertex(p00, pu00, pv00, n00, u, v, m_alpha, m_beta);
            t1[1] = Vertex(p10, pu10, pv10, n10, u_next, v, m_alpha, m_beta);
            t1[2] = Vertex(p01, pu01, pv01, n01, u, v_next, m_alpha, m_beta);

            t2[0] = Vertex(p10, pu10, pv10, n10, u_next, v, m_alpha, m_beta);
            t2[1] = Vertex(p11, pu11, pv11, n11, u_next, v_next, m_alpha, m_beta);
            t2[2] = Vertex(p01, pu01, pv01, n01, u, v_next, m_alpha, m_beta);

            arr.push_back(t1);
            arr.push_back(t2);
        }
    }

    return arr;
}

BernsteinTable Mesh::_computeBernstein(const float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    const float mt = 1.0f - t;
    const float mt2 = mt * mt;
    const float mt3 = mt2 * mt;
    return {
        mt3, // (1-t)^3
        3.0f * mt2 * t, // 3(1-t)^2t
        3.0f * mt * t2, // 3(1-t)t^2
        t3 // t^3
    };
}

std::tuple<QVector3D, QVector3D, QVector3D> Mesh::_computePointAndDeriv(const ControlPoints &points,
                                                                        const BernsteinTable &bu,
                                                                        const BernsteinTable &bv) const {
    QVector3D derivativeU{};
    QVector3D derivativeV{};
    QVector3D point{};
    static constexpr std::array<float, 4> derivativeCoeffs = {-3.0f, -6.0f, 3.0f, 6.0f};

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            derivativeV += points[i * 4 + j] * (derivativeCoeffs[i] * bv[j]);
            derivativeU += points[i * 4 + j] * (bu[i] * derivativeCoeffs[j]);
            point += points[i * 4 + j] * (bu[i] * bv[j]);
        }
    }
    return {point, derivativeU, derivativeV};
}

void Mesh::rotate(QVector3D &p, const float xRotationAngle, const float zRotationAngle) {
    const float x = p.x();
    float y = p.y();
    const float z = p.z();

    const float zRad = zRotationAngle * static_cast<float>(M_PI) / 180.0f;
    const float xRad = xRotationAngle * static_cast<float>(M_PI) / 180.0f;

    /* Z rotation */
    p.setX(x * std::cos(xRad) - y * std::sin(xRad));
    y = x * std::sin(xRad) + y * std::cos(xRad);

    /* x rotation */
    p.setY(y * std::cos(zRad) - z * std::sin(zRad));
    p.setZ(y * std::sin(zRad) + z * std::cos(zRad));
}

void Mesh::setControlPoints(const ControlPoints &controlPoints) {
    m_controlPoints = controlPoints;
    m_triangles = _interpolateBezier(m_controlPoints);
}
