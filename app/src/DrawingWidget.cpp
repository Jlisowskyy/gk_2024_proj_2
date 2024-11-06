//
// Created by Jlisowskyy on 11/04/24.
//

/* internal includes */
#include "../include/GraphicObjects/DrawingWidget.h"
#include "../include/GraphicObjects/BezierPoint3D.h"
#include "../include/ManagingObjects/ObjectMgr.h"

/* external includes */
#include <QDebug>
#include <QShortcut>
#include <QVector3D>
#include <ranges>
#include <QPixmap>

DrawingWidget::DrawingWidget(QWidget *parent) : QGraphicsView(parent),
                                                m_scene(new QGraphicsScene(this)),
                                                m_observerDistance(DEFAULT_OBSERVER_DISTANCE) {
    Q_ASSERT(parent != nullptr);
    setScene(m_scene);
    m_scene->setBackgroundBrush(Qt::white);

    /* disable scrolls visibility */
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* Additional space options */
    setViewportUpdateMode(FullViewportUpdate);
    setRenderHint(QPainter::Antialiasing, true);
    setTransformationAnchor(AnchorViewCenter);

    updateScene();
}

DrawingWidget::~DrawingWidget() {
    clearContent();
}

void DrawingWidget::clearContent() {
    m_scene->clear();
    m_points.clear();
    m_lines.clear();
    m_triangleLines.clear();
}

void DrawingWidget::drawBezierPoint(const QVector3D &point) {
    m_points.push_back(point);
}

void DrawingWidget::drawBezierLine(const QVector3D &start, const QVector3D &end) {
    m_lines.emplace_back(start, end);
}

void DrawingWidget::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    updateScene();
}

void DrawingWidget::setColor(const QColor &color) {
    if (m_color == color) {
        return;
    }

    m_color = color;

    if (m_fillType == FillType::SIMPLE_COLOR) {
        updateScene();
    }
}

void DrawingWidget::setFillType(FillType fillType) {
    if (m_fillType == fillType) {
        return;
    }

    m_fillType = fillType;
    updateScene();
}

void DrawingWidget::updateScene() {
    const QRectF viewRect = rect();
    const qreal width = viewRect.width();
    const qreal height = viewRect.height();

    setSceneRect(-width / 2, -height / 2, width, height);
    updateElements();
}

void DrawingWidget::updateElements() {
    const QRectF viewRect = rect();
    const qreal width = viewRect.width();
    const qreal height = viewRect.height();

    m_scene->clear();
    m_pixMap = new QPixmap(static_cast<int>(width), static_cast<int>(height));
    m_pixMap->fill(Qt::white);

    size_t idx = 0;
    for (const auto &point: m_points) {
        _drawBezierPoint(point, idx++);
    }

    for (const auto &line: m_lines) {
        _drawBezierLine(line);
    }

    for (const auto &line: m_triangleLines) {
        _drawTriangleLine(line);
    }

    if (m_triangles) {
        for (const auto &triangle: *m_triangles) {
            _fillTriangle(triangle);
        }
    }

    const auto pMapItem = m_scene->addPixmap(*m_pixMap);
    pMapItem->setZValue(-1);
    pMapItem->setPos(QPointF(-width / 2, -height / 2));
}

void DrawingWidget::_drawBezierPoint(const QVector3D &point, const size_t idx) const {
    auto *pointItem = new BezierPoint3DItem(point, DEFAULT_BEZIER_POINT_RADIUS, idx);
    m_scene->addItem(pointItem);
    pointItem->setPos(dropPointToScreen(point));
    pointItem->setZValue(2);
}

void DrawingWidget::setObserverDistance(const double distance) {
    m_observerDistance = distance;
    updateScene();
}

void DrawingWidget::_drawBezierLine(const std::pair<QVector3D, QVector3D> &line) const {
    const auto start = dropPointToScreen(line.first);
    const auto end = dropPointToScreen(line.second);
    const auto pLine = m_scene->addLine(start.x(), start.y(), end.x(), end.y());

    pLine->setZValue(1);
    QPen pen(DEFAULT_BEZIER_LINE_COLOR);
    pen.setWidth(3);
    pLine->setPen(pen);
}

QPointF DrawingWidget::dropPointToScreen(const QVector3D &point) const {
    //    const double z = point.z() + m_observerDistance;
    //    const double projX = (point.x() * m_observerDistance / z);
    //    const double projY = (-point.y() * m_observerDistance / z);
    //
    //    return {projX, projY};

    return {point.x(), -point.y()};
}

void DrawingWidget::setTriangles(std::vector<Triangle> *triangles) {
    m_triangles = triangles;
}

void DrawingWidget::drawTriangleLines(const QVector3D &start, const QVector3D &end) {
    m_triangleLines.emplace_back(start, end);
}

void DrawingWidget::_drawTriangleLine(const std::pair<QVector3D, QVector3D> &line) const {
    const auto start = dropPointToScreen(line.first);
    const auto end = dropPointToScreen(line.second);
    const auto pLine = m_scene->addLine(start.x(), start.y(), end.x(), end.y());

    pLine->setZValue(0);
    auto pen = QPen(DEFAULT_TRIANGLE_LINE_COLOR);
    pen.setWidth(1);
    pLine->setPen(pen);
}

void DrawingWidget::_fillTriangle(const Triangle &triangle) {
    switch (m_fillType) {
        case FillType::TEXTURE:
        case FillType::SIMPLE_COLOR: {
            auto curColor = m_color;
            colorPolygon(
                [=](const QVector3D &pos) {
                    return curColor;
                },
                [](const Vertex &vertex, const QColor &color) {
                    return color;
                },
                triangle
            );
        }
        break;
        default:
            Q_ASSERT(false);
    }
}
