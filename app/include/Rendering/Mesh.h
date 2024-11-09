//
// Created by Jlisowskyy on 11/8/24.
//

#ifndef MESH_H
#define MESH_H


/* internal includes */
#include "../Intf.h"

/* external includes */
#include <QObject>
#include <Qt>

class Mesh : public QObject {
    Q_OBJECT
    // ------------------------------
    // Class creation
    // ------------------------------
public:
    explicit Mesh(QObject *parent, const ControlPoints &controlPoints, float alpha, float beta, int accuracy);

    ~Mesh() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] const ControlPoints &getControlPoints() const {
        return m_controlPoints;
    }

    [[nodiscard]] const MeshArr &getMeshArr() const {
        return m_triangles;
    }

    static void rotate(QVector3D &p, float xRotationAngle, float zRotationAngle);

    void alignWithMeshPlain(QVector3D &p) const { rotate(p, m_alpha, m_beta); }

    [[nodiscard]] QVector3D getPointAlignedWithMeshPlain(const QVector3D &p) const {
        QVector3D pAligned = p;
        alignWithMeshPlain(pAligned);
        return pAligned;
    }

    void setControlPoints(const ControlPoints &controlPoints);

    // ------------------------------
    // Public slots
    // ------------------------------
public slots:
    void setAlpha(double alpha);

    void setBeta(double beta);

    void setAccuracy(double accuracy);

    // ------------------------------
    // Class protected methods
    // ------------------------------
protected:
    [[nodiscard]] MeshArr _interpolateBezier(const ControlPoints &controlPoints) const;

    [[nodiscard]] static std::tuple<BernsteinTable, BernsteinTable> _computeBernstein(float t);

    [[nodiscard]] static std::tuple<QVector3D, QVector3D, QVector3D> _computePointAndDeriv(
        const ControlPoints &points,
        const BernsteinTable &bu,
        const BernsteinTable &bv,
        const BernsteinTable &buDeriv,
        const BernsteinTable &bvDeriv);

    void _adjustAfterRotation();

    // ------------------------------
    // Class fields
    // ------------------------------

    int m_triangleAccuracy;
    float m_alpha;
    float m_beta;

    ControlPoints m_controlPoints;
    MeshArr m_triangles;
};

#endif //MESH_H
