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
                                                m_timer(new QTimer(this)),
                                                m_observerDistance(VIEW_SETTINGS::DEFAULT_OBSERVER_DISTANCE) {
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
    _setupLight();
    _processLightPosition();
}

DrawingWidget::~DrawingWidget() {
    clearContent();
}

void DrawingWidget::clearContent() {
    m_scene->clear();
    m_points.clear();
    m_lines.clear();
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

void DrawingWidget::updateScene() {
    const QRectF viewRect = rect();
    m_width = static_cast<float>(viewRect.width());
    m_height = static_cast<float>(viewRect.height());

    setSceneRect(-m_width / 2, -m_height / 2, m_width, m_height);
    updateElements();
}

void DrawingWidget::updateElements() {
    m_scene->clear();
    m_pixMap = new QPixmap(static_cast<int>(m_width), static_cast<int>(m_height));

    _addLightDrawing();

    size_t idx = 0;
    for (const auto &point: m_points) {
        _drawBezierPoint(point, idx++);
    }

    for (const auto &line: m_lines) {
        _drawBezierLine(line);
    }

    const auto pMapItem = m_scene->addPixmap(*m_pixMap);
    pMapItem->setZValue(-1);
    pMapItem->setPos(QPointF(-m_width / 2, -m_height / 2));
    m_pixMapItem = pMapItem;

    _drawTexture();
}

void DrawingWidget::_drawBezierPoint(const QVector3D &point, const size_t idx) const {
    auto *pointItem = new BezierPoint3DItem(point, BEZIER_CONSTANTS::DEFAULT_POINT_RADIUS, idx);
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
    QPen pen(UI_CONSTANTS::DEFAULT_BEZIER_LINE_COLOR);
    pen.setWidth(3);
    pLine->setPen(pen);
}

QPointF DrawingWidget::dropPointToScreen(const QVector3D &point) const {
    //    const double z = point.z() + m_observerDistance;
    //    const double projX = (point.x() * m_observerDistance / z);
    //    const double projY = (-point.y() * m_observerDistance / z);
    //
    //    return {projX, projY};

    return {point.x(), point.y()};
}

void DrawingWidget::_drawTriangleLine(const std::pair<QVector3D, QVector3D> &line) const {
    const auto start = dropPointToScreen(line.first);
    const auto end = dropPointToScreen(line.second);
    const auto pLine = m_scene->addLine(start.x(), start.y(), end.x(), end.y());

    pLine->setZValue(0);
    auto pen = QPen(UI_CONSTANTS::DEFAULT_TRIANGLE_LINE_COLOR);
    pen.setWidth(1);
    pLine->setPen(pen);
}

// void DrawingWidget::_fillTriangle(const Triangle &triangleToFill) {
//     switch (m_fillType) {
//         case FillType::TEXTURE: {
//             colorPolygon(
//                 [this](const QVector3D &pos, const Triangle &triangle) {
//                     return _getTextureColor(pos, triangle);
//                 },
//                 triangleToFill
//             );
//         }
//         break;
//         case FillType::SIMPLE_COLOR: {
//             colorPolygon(
//                     [this](const QVector3D &pos, const Triangle &triangle) {
//                         return _getPlainColor(pos, triangle);
//                 },
//                 triangleToFill
//             );
//         }
//         break;
//         default:
//             Q_ASSERT(false);
//     }
// }

// QColor DrawingWidget::_getTextureColor(const QVector3D &pos, const Triangle &triangle) const {
//     const auto [interpolatedU, interpolatedV, interpolatedNormalVector] = _interpolateFromTrianglePoint(pos, triangle);
//     const QColor color = m_texture->pixelColor(
//             static_cast<int>(interpolatedU * static_cast<float>(m_texture->width() - 1)),
//             static_cast<int>(interpolatedV * static_cast<float>(m_texture->height() - 1))
//     );
//
//     return _applyLightToTriangleColor(color, interpolatedNormalVector, pos);
// }
//
// QColor DrawingWidget::_getPlainColor(const QVector3D &pos, const Triangle &triangle) const {
//     const auto [u0, u1, interpolatedNormalVector] = _interpolateFromTrianglePoint(pos, triangle);
//     return _applyLightToTriangleColor(m_color, interpolatedNormalVector, pos);
// }


void DrawingWidget::_setupLight() {
    connect(m_timer, &QTimer::timeout, this, &DrawingWidget::_onTimer);

    m_timer->start(LIGHTING_CONSTANTS::ANIMATION_TIME_STEP_MS);
}

void DrawingWidget::setLightZ(const int value) {
    m_lightZ = value;

    if (m_stopLight) {
        _drawTexture();
    }
}

void DrawingWidget::_onTimer() {
    if (m_stopLight) {
        return;
    }

    m_lightPos += std::fmod(
        LIGHTING_CONSTANTS::LIGHT_MOVEMENT_STEP, 1.0f);

    _processLightPosition();
    _drawTexture();
}

void DrawingWidget::_processLightPosition() {
    const auto point = _getLightPosition2D();

    m_lightEllipse->setRect(
        point.x() - UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        point.y() - UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        2 * UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        2 * UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS
    );
}

// void DrawingWidget::_drawTexture() {
//     m_pixMap->fill(Qt::white);
//
//     if (m_triangles) {
//         // #pragma omp parallel for schedule(static)
//         for (const auto &triangle: *m_triangles) {
//             _fillTriangle(triangle);
//         }
//     }
//
//     m_pixMapItem->setPixmap(*m_pixMap);
// }

QPointF DrawingWidget::_getLightPosition2D() const {
    const float spiralRadius = UI_CONSTANTS::DEFAULT_LIGHT_MOVE_RADIUS * m_lightPos;
    const float radian = 2.0f * static_cast<float>(M_PI) * LIGHTING_CONSTANTS::NUMBER_OF_SPIRALS * m_lightPos;

    const float x = spiralRadius * std::cos(radian);
    const float y = spiralRadius * std::sin(radian);

    return {x, y};
}

void DrawingWidget::_addLightDrawing() {
    const auto point = _getLightPosition2D();

    m_lightEllipse = m_scene->addEllipse(
        point.x() - UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        point.y() - UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        2 * UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        2 * UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        QPen(UI_CONSTANTS::LIGHT_SOURCE_COLOR),
        QBrush(UI_CONSTANTS::LIGHT_SOURCE_COLOR)
    );
}

QVector3D DrawingWidget::_getLightPosition3D() const {
    const QPointF point2D = _getLightPosition2D();
    return {static_cast<float>(point2D.x()), static_cast<float>(point2D.y()), static_cast<float>(m_lightZ)};
}

void DrawingWidget::setStopLight(const bool value) {
    m_stopLight = value;
}
