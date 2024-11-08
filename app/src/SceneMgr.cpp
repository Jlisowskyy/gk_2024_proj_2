//
// Created by Jlisowskyy on 11/8/24.
//

/* internal includes */
#include "../include/ManagingObjects/SceneMgr.h"
#include "../include/Rendering/Mesh.h"
#include "../include/Rendering/Texture.h"
#include "../include/GraphicObjects/DrawingWidget.h"

/* external includes */
SceneMgr::SceneMgr(QObject *parent,
                   const QColor &color,
                   const bool drawNet,
                   const bool useTexture,
                   const bool playAnimation,
                   QImage *image) : QObject(parent),
                                    m_useTexture(useTexture),
                                    m_isAnimationPlayed(playAnimation),
                                    m_drawNet(drawNet),
                                    m_fillType(getFillType()),
                                    m_color(color),
                                    m_timer(new QTimer(this)) {
}

FillType SceneMgr::getFillType() const {
    return m_useTexture && m_textureImg ? FillType::TEXTURE : FillType::SIMPLE_COLOR;
}

void SceneMgr::redrawScene(DrawingWidget &drawingWidget, const Texture &texture, const Mesh &mesh) {
    drawingWidget.clearContent();

    if (m_drawNet) {
        _drawNet(drawingWidget, mesh);
    }
    drawingWidget.updateScene();
    _drawTexture(drawingWidget, texture, mesh);
}

void SceneMgr::setupLightAnim(DrawingWidget *drawingWidget, Texture *texture, Mesh *mesh) {
    Q_ASSERT(drawingWidget && texture && mesh);
    Q_ASSERT(!m_texture && !m_mesh && !m_drawingWidget);
    m_texture = texture;
    m_mesh = mesh;
    m_drawingWidget = drawingWidget;

    connect(drawingWidget, DrawingWidget::onElementsUpdate, this, &SceneMgr::_addLightItem);
    _addLightItem(drawingWidget);

    connect(m_timer, &QTimer::timeout, this, &SceneMgr::_onTimer);
    m_timer->start(LIGHTING_CONSTANTS::ANIMATION_TIME_STEP_MS);
}

void SceneMgr::_onTimer() {
    if (!m_isAnimationPlayed) {
        return;
    }

    m_lightPos += std::fmod(
        LIGHTING_CONSTANTS::LIGHT_MOVEMENT_STEP, 1.0f);

    _processLightPosition();
    _drawTexture(*m_drawingWidget, *m_texture, *m_mesh);
}

void SceneMgr::_addLightItem(const DrawingWidget *sender) {
    const auto point = _getLightPosition2D();

    m_lightEllipse = sender->scene()->addEllipse(
        point.x() - UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        point.y() - UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        2 * UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        2 * UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        QPen(UI_CONSTANTS::LIGHT_SOURCE_COLOR),
        QBrush(UI_CONSTANTS::LIGHT_SOURCE_COLOR)
    );
}

void SceneMgr::_drawNet(DrawingWidget &drawingWidget, const Mesh &mesh) {
    /* Draw control points */
    for (auto point: mesh.getControlPoints()) {
        drawingWidget.drawBezierPoint(mesh.getPointAlignedWithMeshPlain(point));
    }

    /* Draw lines for control points */
    static constexpr int CONTROL_POINTS_MATRIX_SIZE_INT = BEZIER_CONSTANTS::CONTROL_POINTS_MATRIX_SIZE;
    static constexpr int CONTROL_POINTS_COUNT_INT = BEZIER_CONSTANTS::CONTROL_POINTS_COUNT;
    for (int i = 0; i < CONTROL_POINTS_COUNT_INT - 1; i++) {
        const int row = i / CONTROL_POINTS_MATRIX_SIZE_INT;
        const int col = i % CONTROL_POINTS_MATRIX_SIZE_INT;

        static constexpr int dx[] = {0, 1};
        static constexpr int dy[] = {1, 0};

        for (int j = 0; j < 2; j++) {
            const int newRow = row + dx[j];
            const int newCol = col + dy[j];

            if (newRow >= CONTROL_POINTS_MATRIX_SIZE_INT || newCol >= CONTROL_POINTS_MATRIX_SIZE_INT) {
                continue;
            }

            const int idx = newRow * CONTROL_POINTS_MATRIX_SIZE_INT + newCol;
            auto point1 = mesh.getControlPoints()[i];
            auto point2 = mesh.getControlPoints()[idx];

            drawingWidget.drawBezierLine(mesh.getPointAlignedWithMeshPlain(point1),
                                         mesh.getPointAlignedWithMeshPlain(point2));
        }
    }
}

void SceneMgr::_drawTexture(const DrawingWidget &drawingWidget, const Texture &texture, const Mesh &mesh) {
    switch (m_fillType) {
        case FillType::TEXTURE: {
            texture.fillPixmap(*drawingWidget.getPixMap(), mesh,
                               [this](const int u, const int v) {
                                   return m_textureImg->pixelColor(
                                       static_cast<int>(u * static_cast<float>(m_textureImg->width() - 1)),
                                       static_cast<int>(v * static_cast<float>(m_textureImg->height() - 1))
                                   );
                               },
                               _getLightPos()
            );
        }
        break;
        case FillType::SIMPLE_COLOR: {
            texture.fillPixmap(*drawingWidget.getPixMap(), mesh,
                               [this]([[maybe_unused]] const int u, [[maybe_unused]] const int v) {
                                   return m_color;
                               },
                               _getLightPos()
            );
        }
        break;
        default:
            Q_ASSERT(false);
    }
    drawingWidget.setPixmap(drawingWidget.getPixMap());
}

void SceneMgr::_processLightPosition() {
    const auto point = _getLightPosition2D();

    m_lightEllipse->setRect(
        point.x() - UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        point.y() - UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        2 * UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS,
        2 * UI_CONSTANTS::DEFAULT_LIGHT_SOURCE_RADIUS
    );
}

QPointF SceneMgr::_getLightPosition2D() const {
    const float spiralRadius = UI_CONSTANTS::DEFAULT_LIGHT_MOVE_RADIUS * m_lightPos;
    const float radian = 2.0f * static_cast<float>(M_PI) * LIGHTING_CONSTANTS::NUMBER_OF_SPIRALS * m_lightPos;

    const float x = spiralRadius * std::cos(radian);
    const float y = spiralRadius * std::sin(radian);

    return {x, y};
}

QVector3D SceneMgr::_getLightPos() const {
    const QPointF point2D = _getLightPosition2D();
    return {static_cast<float>(point2D.x()), static_cast<float>(point2D.y()), static_cast<float>(m_lightZ)};
}
