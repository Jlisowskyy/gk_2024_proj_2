//
// Created by Jlisowskyy on 11/04/24.
//

/* internal includes */
#include "../include/GraphicObjects/DrawingWidget.h"
#include "../include/GraphicObjects/BezierPoint3D.h"

/* external includes */
#include <QDebug>
#include <format>
#include <QShortcut>
#include <QVector3D>
#include <ranges>

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
}

DrawingWidget::~DrawingWidget() {
    clearContent();
}

void DrawingWidget::clearContent() {
    m_scene->clear();
    m_points.clear();
}

void DrawingWidget::drawBezierPoint(QVector3D point) {
    m_points.push_back(point);
    _drawBezierPoint(point, m_points.size() - 1);
}

void DrawingWidget::drawBezierLine(QVector3D start, QVector3D end) {
    m_lines.emplace_back(start, end);
    _drawBezierLine({start, end});
}

void DrawingWidget::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    updateScene();
}

void DrawingWidget::updateScene() {
    QRectF viewRect = rect();
    qreal width = viewRect.width();
    qreal height = viewRect.height();

    setSceneRect(-width / 2, -height / 2, width, height);
    updateElements();
}

void DrawingWidget::updateElements() {
    m_scene->clear();

    size_t idx = 0;
    for (const auto& point : m_points) {
        _drawBezierPoint(point, idx++);
    }

    for (const auto &line: m_lines) {
        _drawBezierLine(line);
    }
}

void DrawingWidget::_drawBezierPoint(const QVector3D &point, const size_t idx) {
    auto *pointItem = new BezierPoint3DItem(point, DEFAULT_BEZIER_POINT_RADIUS, idx);
    m_scene->addItem(pointItem);
    pointItem->setPos(dropPointToScreen(point));
}

void DrawingWidget::setObserverDistance(double distance) {
    m_observerDistance = distance;
    updateScene();
}

void DrawingWidget::_drawBezierLine(const std::pair<QVector3D, QVector3D> &line) {
    const auto start = dropPointToScreen(line.first);
    const auto end = dropPointToScreen(line.second);
    m_scene->addLine(start.x(), start.y(), end.x(), end.y());
}

QPointF DrawingWidget::dropPointToScreen(const QVector3D &point) const {
    const double z = point.z() + m_observerDistance;
    const double projX = (point.x() * m_observerDistance / z);
    const double projY = (-point.y() * m_observerDistance / z);

    return {projX, projY};
}
