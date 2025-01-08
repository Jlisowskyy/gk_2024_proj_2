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
    explicit Mesh(QObject *parent, const ControlPoints &controlPoints, float alpha, float beta, float delta,
                  int accuracy);

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

    [[nodiscard]] const MeshArr &getFigure() const {
        return m_figure;
    }

    static void rotate(QVector3D &p, float xRotationAngle, float zRotationAngle, float yRotationAngle);

    void alignWithMeshPlain(QVector3D &p) const { rotate(p, m_alpha, m_beta, m_delta); }

    [[nodiscard]] QVector3D getPointAlignedWithMeshPlain(const QVector3D &p) const {
        QVector3D pAligned = p;
        alignWithMeshPlain(pAligned);
        return pAligned;
    }

    void setControlPoints(const ControlPoints &controlPoints);

    void rotateFigure();

    QColor getFigureColor(size_t idx) const;

    // ------------------------------
    // Public slots
    // ------------------------------
public slots:
    void setAlpha(double alpha);

    void setBeta(double beta);

    void setDelta(double delta);

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

    static MeshArr _getFigure();

    // ------------------------------
    // Class fields
    // ------------------------------

    int m_triangleAccuracy;
    float m_alpha;
    float m_beta;
    float m_delta;

    ControlPoints m_controlPoints;
    MeshArr m_triangles;
    MeshArr m_figure;
};

#endif //MESH_H
