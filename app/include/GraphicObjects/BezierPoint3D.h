//
// Created by Jlisowskyy on 05/11/24.
//

#ifndef APP_BEZIERPOINT3D_H
#define APP_BEZIERPOINT3D_H

/* external includes */
#include <QGraphicsEllipseItem>
#include <QVector3D>
#include <Qt>
#include <QBrush>
#include <QPen>

class BezierPoint3DItem : public QGraphicsEllipseItem {
public:
    // ------------------------------
    // Class creation
    // ------------------------------

    BezierPoint3DItem(const QVector3D &point3d, double size, size_t idx, QGraphicsItem *parent = nullptr);

    ~BezierPoint3DItem() override = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] QVector3D get3DPoint() const { return m_point; }

    // ------------------------------
    // Class fields
    // ------------------------------

private:
    QVector3D m_point;
};


#endif //APP_BEZIERPOINT3D_H
