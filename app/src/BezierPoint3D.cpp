//
// Created by Jlisowskyy on 05/11/24.
//

/* internal includes */
#include "../include/GraphicObjects/BezierPoint3D.h"
#include "../include/Constants.h"

/* external includes */
#include <QDebug>
#include <QFont>

BezierPoint3DItem::BezierPoint3DItem(const QVector3D &point3d, const double size, const size_t idx,
                                     QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent), m_point(point3d) {
    setRect(-size / 2, -size / 2, size, size);
    setBrush(UI_CONSTANTS::DEFAULT_BEZIER_POINT_COLOR);
    setPen(QPen(UI_CONSTANTS::DEFAULT_BEZIER_POINT_COLOR));

    auto *label = new QGraphicsTextItem(QString::number(idx), this);

    QFont font = label->font();
    font.setPointSize(10);
    label->setFont(font);

    label->setPos(-size / 2 - 10, -size / 2 - 20);
    label->setDefaultTextColor(UI_CONSTANTS::DEFAULT_BEZIER_POINT_COLOR);
}
