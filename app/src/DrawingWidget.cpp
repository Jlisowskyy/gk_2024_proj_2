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

    Q_ASSERT(m_fillType != FillType::TEXTURE || m_texture);
    updateScene();
}

void DrawingWidget::updateScene() {
    const QRectF viewRect = rect();
    m_width = viewRect.width();
    m_height = viewRect.height();

    setSceneRect(-m_width / 2, -m_height / 2, m_width, m_height);
    updateElements();
}

void DrawingWidget::updateElements() {
    m_scene->clear();
    m_pixMap = new QPixmap(static_cast<int>(m_width), static_cast<int>(m_height));
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
    pMapItem->setPos(QPointF(-m_width / 2, -m_height / 2));
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

    return {point.x(), point.y()};
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
        case FillType::TEXTURE: {
            colorPolygon(
                    [this](const QVector3D &pos, const Triangle &triangle) {
                        return _getTextureColor(pos, triangle);
                    },
                    [](const Vertex &vertex, const QColor &color) {
                        return color;
                    },
                    triangle
            );
        }
        break;
        case FillType::SIMPLE_COLOR: {
            auto curColor = m_color;
            colorPolygon(
                    [=](const QVector3D &pos, const Triangle &triangle) {
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

void DrawingWidget::setTexture(QImage *texture) {
    m_texture = texture;
}

QColor DrawingWidget::_getTextureColor(const QVector3D &pos, const Triangle &triangle) {
    const QVector2D v0(
            triangle[1].rotatedPosition.x() - triangle[0].rotatedPosition.x(),
            triangle[1].rotatedPosition.y() - triangle[0].rotatedPosition.y()
    );
    const QVector2D v1(
            triangle[2].rotatedPosition.x() - triangle[0].rotatedPosition.x(),
            triangle[2].rotatedPosition.y() - triangle[0].rotatedPosition.y()
    );
    const QVector2D v2(
            pos.x() - triangle[0].rotatedPosition.x(),
            pos.y() - triangle[0].rotatedPosition.y()
    );

    const float d00 = QVector2D::dotProduct(v0, v0);
    const float d01 = QVector2D::dotProduct(v0, v1);
    const float d11 = QVector2D::dotProduct(v1, v1);
    const float d20 = QVector2D::dotProduct(v2, v0);
    const float d21 = QVector2D::dotProduct(v2, v1);

    const float denom = d00 * d11 - d01 * d01;
    const float v = (d11 * d20 - d01 * d21) / denom;
    const float w = (d00 * d21 - d01 * d20) / denom;
    const float u = 1.0f - v - w;

    float interpolatedU = u * triangle[0].u + v * triangle[1].u + w * triangle[2].u;
    float interpolatedV = u * triangle[0].v + v * triangle[1].v + w * triangle[2].v;
    interpolatedU = std::clamp(interpolatedU, 0.0f, 1.0f);
    interpolatedV = std::clamp(interpolatedV, 0.0f, 1.0f);

    return m_texture->pixelColor(
            static_cast<int>(interpolatedU * static_cast<float>(m_texture->width() - 1)),
            static_cast<int>(interpolatedV * static_cast<float>(m_texture->height() - 1))
    );
}
