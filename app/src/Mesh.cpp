//
// Created by Jlisowskyy on 11/8/24.
//

/* Main header */
#include "../include/Rendering/Mesh.h"

/* external includes */
#include <cmath>


Mesh::Mesh(QObject *parent, const ControlPoints &controlPoints, const float alpha, const float beta, const float delta,
           const int accuracy): QObject(parent),
                                m_triangleAccuracy(accuracy),
                                m_alpha(alpha),
                                m_beta(beta),
                                m_delta(delta),
                                m_controlPoints(controlPoints),
                                m_triangles(_interpolateBezier(controlPoints)) {
}

void Mesh::setAlpha(const double alpha) {
    m_alpha = static_cast<float>(alpha);
    _adjustAfterRotation();
}

void Mesh::setBeta(const double beta) {
    m_beta = static_cast<float>(beta);
    _adjustAfterRotation();
}

void Mesh::setDelta(const double delta) {
    m_delta = static_cast<float>(delta);
    _adjustAfterRotation();
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

            const auto [bu, buDeriv] = _computeBernstein(u);
            const auto [bv, bvDeriv] = _computeBernstein(v);
            const auto [bu_next, buDeriv_next] = _computeBernstein(u_next);
            const auto [bv_next, bvDeriv_next] = _computeBernstein(v_next);

            const auto [p00, pu00, pv00] = _computePointAndDeriv(controlPoints, bu, bv, buDeriv, bvDeriv);
            const auto [p10, pu10, pv10] = _computePointAndDeriv(controlPoints, bu_next, bv, buDeriv_next, bvDeriv);
            const auto [p01, pu01, pv01] = _computePointAndDeriv(controlPoints, bu, bv_next, buDeriv, bvDeriv_next);
            const auto [p11, pu11, pv11] = _computePointAndDeriv(controlPoints, bu_next, bv_next, buDeriv_next,
                                                                 bvDeriv_next);

            const QVector3D n00 = QVector3D::crossProduct(pu00, pv00).normalized();
            const QVector3D n10 = QVector3D::crossProduct(pu10, pv10).normalized();
            const QVector3D n01 = QVector3D::crossProduct(pu01, pv01).normalized();
            const QVector3D n11 = QVector3D::crossProduct(pu11, pv11).normalized();

            Triangle t1, t2;

            t1[0] = Vertex(p00, pu00, pv00, n00, u, v, m_alpha, m_beta, m_delta);
            t1[1] = Vertex(p10, pu10, pv10, n10, u_next, v, m_alpha, m_beta, m_delta);
            t1[2] = Vertex(p01, pu01, pv01, n01, u, v_next, m_alpha, m_beta, m_delta);

            t2[0] = Vertex(p10, pu10, pv10, n10, u_next, v, m_alpha, m_beta, m_delta);
            t2[1] = Vertex(p11, pu11, pv11, n11, u_next, v_next, m_alpha, m_beta, m_delta);
            t2[2] = Vertex(p01, pu01, pv01, n01, u, v_next, m_alpha, m_beta, m_delta);

            arr.push_back(t1);
            arr.push_back(t2);
        }
    }

    return arr;
}

std::tuple<QVector3D, QVector3D, QVector3D> Mesh::_computePointAndDeriv(
    const ControlPoints &points,
    const BernsteinTable &bu,
    const BernsteinTable &bv,
    const BernsteinTable &buDeriv,
    const BernsteinTable &bvDeriv
) {
    QVector3D point{};
    QVector3D derivativeU{};
    QVector3D derivativeV{};

    for (int i = 0; i < BEZIER_CONSTANTS::CONTROL_POINTS_DIM; ++i) {
        for (int j = 0; j < BEZIER_CONSTANTS::CONTROL_POINTS_DIM; ++j) {
            point += points[i * BEZIER_CONSTANTS::CONTROL_POINTS_DIM + j] * (bu[i] * bv[j]);
        }
    }

    for (int i = 0; i < BEZIER_CONSTANTS::CONTROL_POINTS_DIM - 1; ++i) {
        for (int j = 0; j < BEZIER_CONSTANTS::CONTROL_POINTS_DIM; ++j) {
            const QVector3D &current = points[i * BEZIER_CONSTANTS::CONTROL_POINTS_DIM + j];
            const QVector3D &next = points[(i + 1) * BEZIER_CONSTANTS::CONTROL_POINTS_DIM + j];
            derivativeU += (next - current) * (buDeriv[i] * bv[j]);
        }
    }

    for (int i = 0; i < BEZIER_CONSTANTS::CONTROL_POINTS_DIM; ++i) {
        for (int j = 0; j < BEZIER_CONSTANTS::CONTROL_POINTS_DIM - 1; ++j) {
            const QVector3D &current = points[i * BEZIER_CONSTANTS::CONTROL_POINTS_DIM + j];
            const QVector3D &next = points[i * BEZIER_CONSTANTS::CONTROL_POINTS_DIM + j + 1];
            derivativeV += (next - current) * (bu[i] * bvDeriv[j]);
        }
    }

    derivativeU *= static_cast<float>(BEZIER_CONSTANTS::CONTROL_POINTS_DIM - 1);
    derivativeV *= static_cast<float>(BEZIER_CONSTANTS::CONTROL_POINTS_DIM - 1);

    return {point, derivativeU, derivativeV};
}

void Mesh::_adjustAfterRotation() {
    #pragma omp parallel for
    for (auto &triangle: m_triangles) {
        for (auto &vertex: triangle) {
            vertex.resetRotation();
            vertex.rotate(m_alpha, m_beta, m_delta);
        }
    }
}

void Mesh::rotate(QVector3D &p, const float xRotationAngle, const float zRotationAngle, const float yRotationAngle) {
    const float xRad = xRotationAngle * static_cast<float>(M_PI) / 180.0f;
    const float yRad = yRotationAngle * static_cast<float>(M_PI) / 180.0f;
    const float zRad = zRotationAngle * static_cast<float>(M_PI) / 180.0f;

    float x = p.x();
    float y = p.y();
    float z = p.z();

    /* x rotation */
    y = y * std::cos(xRad) - z * std::sin(xRad);
    z = y * std::sin(xRad) + z * std::cos(xRad);

    /* y rotation */
    x = x * std::cos(yRad) + z * std::sin(yRad);
    z = -x * std::sin(yRad) + z * std::cos(yRad);

    /* z rotation */
    x = x * std::cos(zRad) - y * std::sin(zRad);
    y = x * std::sin(zRad) + y * std::cos(zRad);

    p.setX(x);
    p.setY(y);
    p.setZ(z);
}

void Mesh::setControlPoints(const ControlPoints &controlPoints) {
    m_controlPoints = controlPoints;
    m_triangles = _interpolateBezier(m_controlPoints);
}

std::tuple<BernsteinTable, BernsteinTable> Mesh::_computeBernstein(const float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    const float mt = 1.0f - t;
    const float mt2 = mt * mt;
    const float mt3 = mt2 * mt;
    return {
        {
            mt3,
            3.0f * mt2 * t,
            3.0f * mt * t2,
            t3,
        },
        {
            mt2,
            2.0f * t * mt,
            t2,
            0.0f,
        }
    };
}
