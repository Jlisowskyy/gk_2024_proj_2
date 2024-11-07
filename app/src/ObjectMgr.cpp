//
// Created by Jlisowskyy on 11/5/24.
//

/* internal includes */
#include "../include/ManagingObjects/ObjectMgr.h"
#include "../include/ManagingObjects/ToolBar.h"
#include "../include/UiObjects/DoubleSlider.h"
#include "../include/GraphicObjects/DrawingWidget.h"

/* external includes */
#include <vector>
#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QVector3D>
#include <QRegularExpression>
#include <array>
#include <QLabel>
#include <QTimer>
#include <cmath>

ObjectMgr::ObjectMgr(QObject *parent, QWidget *widgetParent, DrawingWidget *drawingWidget) : QObject(parent),
    m_parentWidget(widgetParent),
    m_drawingWidget(drawingWidget) {
    Q_ASSERT(parent && widgetParent && drawingWidget);
    m_drawingWidget->setTriangles(&m_triangles);
}

ObjectMgr::~ObjectMgr() = default;

void ObjectMgr::connectToToolBar(ToolBar *toolBar) {
    /* State changes */
    std::vector<std::pair<DoubleSlider *, void (ObjectMgr::*)(double)> > vSliderProc{
        {toolBar->m_triangulationSlider, &ObjectMgr::onTriangulationChanged},
        {toolBar->m_alphaSlider, &ObjectMgr::onAlphaChanged},
        {toolBar->m_betaSlider, &ObjectMgr::onBetaChanged},
        {toolBar->m_ksSlider, &ObjectMgr::onKSChanged},
        {toolBar->m_kdSlider, &ObjectMgr::onKDChanged},
        {toolBar->m_mSlider, &ObjectMgr::onMChanged},
    };

    for (const auto &[slider, proc]: vSliderProc) {
        connect(slider, &DoubleSlider::doubleValueChanged, this, proc);
    }

    /* simple actions */
    std::vector<std::pair<QAction *, void (ObjectMgr::*)()> > vActionProc{
        {toolBar->m_loadTextureButton, &ObjectMgr::onLoadTexturesTriggered},
        {toolBar->m_loadBezierPointsButton, &ObjectMgr::onLoadBezierPointsTriggered},
        {toolBar->m_loadNormalVectorsButton, &ObjectMgr::onLoadNormalVectorsTriggered},
        {toolBar->m_changePlainColorButton, &ObjectMgr::onColorChangedTriggered},
    };

    for (const auto &[action, proc]: vActionProc) {
        connect(action, &QAction::triggered, this, proc);
    }

    /* Check able actions */
    std::vector<std::pair<QAction *, void (ObjectMgr::*)(bool)> > vActionBoolProc{
        {toolBar->m_drawNetButton, &ObjectMgr::onDrawNetChanged},
        {toolBar->m_enableTextureButton, &ObjectMgr::onEnableTextureChanged},
        {toolBar->m_enableNormalVectorsButton, &ObjectMgr::onEnableNormalVectorsChanged},
        {toolBar->m_stopLightMovementButton, &ObjectMgr::onStopLightingMovementChanged},
    };

    for (const auto &[action, proc]: vActionBoolProc) {
        connect(action, &QAction::triggered, this, proc);
    }

    /* Connect to toolbar */
    connect(toolBar->m_observerDistanceSlider,
            &DoubleSlider::doubleValueChanged,
            m_drawingWidget,
            &DrawingWidget::setObserverDistance
    );
}

void ObjectMgr::loadDefaultSettings() {
    m_drawNet = true;
    m_triangleAccuracy = VIEW_SETTINGS::DEFAULT_TRIANGLE_ACCURACY;
    m_alpha = 0;
    m_beta = 0;
    m_drawingWidget->setColor(UI_CONSTANTS::DEFAULT_PLAIN_COLOR);

    _loadBezierPoints(RESOURCE_CONSTANTS::DEFAULT_CONTROL_POINTS_PATH);
    _loadTexture(RESOURCE_CONSTANTS::DEFAULT_TEXTURE_PATH);
    redraw();
}

void ObjectMgr::redraw() {
    m_drawingWidget->clearContent();
    m_triangles.clear();

    _interpolateBezier();

    if (m_drawNet) {
        _drawNet();
    }
    m_drawingWidget->updateScene();
}

void ObjectMgr::onTriangulationChanged(double value) {
    m_triangleAccuracy = static_cast<int>(value);
    redraw();
}

void ObjectMgr::onAlphaChanged(double value) {
    m_alpha = value;
    redraw();
}

void ObjectMgr::onBetaChanged(double value) {
    m_beta = value;
    redraw();
}

void ObjectMgr::onKSChanged(double value) {
}

void ObjectMgr::onKDChanged(double value) {
}

void ObjectMgr::onMChanged(double value) {
}

void ObjectMgr::onDrawNetChanged(const bool isChecked) {
    m_drawNet = isChecked;
    redraw();
}

void ObjectMgr::onEnableTextureChanged(const bool isChecked) {
    m_useTexture = isChecked;
    m_drawingWidget->setFillType(m_texture && m_useTexture ? FillType::TEXTURE : FillType::SIMPLE_COLOR);
}

void ObjectMgr::onEnableNormalVectorsChanged(bool isChecked) {
}

void ObjectMgr::onStopLightingMovementChanged(bool isChecked) {
}

void ObjectMgr::onLoadBezierPointsTriggered() {
    _openFileDialog([this](const QString &path) {
        _loadBezierPoints(path);
                    },
                    "Text Files (*.txt);;All Files (*)");
}

void ObjectMgr::onLoadTexturesTriggered() {
    _openFileDialog([this](const QString &path) {
        _loadTexture(path);
    }, "Images (*.png *.jpg *.bmp);;All Files (*)");
}

void ObjectMgr::onLoadNormalVectorsTriggered() {
    _openFileDialog([](const QString &path) {
        qDebug() << "Loading normal vectors from path:" << path;
    }, "");
}

void ObjectMgr::onColorChangedTriggered() {
    const QColor selectedColor = QColorDialog::getColor(Qt::white, m_parentWidget, "Choose Color");
    if (!selectedColor.isValid()) {
        qDebug() << "Error getting color";
        return;
    }

    m_drawingWidget->setColor(selectedColor);
    redraw();
}

void ObjectMgr::_loadBezierPoints(const QString &path) {
    bool ok;
    const ControlPoints controlPoints = _loadBezierPointsOpenFile(path, &ok);
    if (!ok) {
        qWarning() << "Failed to load bezier points from file:" << path;
        return;
    }

    m_controlPoints = controlPoints;
    redraw();
}

void ObjectMgr::_openFileDialog(const std::function<void(const QString &)> &callback, const char *filter) {
    Q_ASSERT(callback);

    QString initialPath = m_previousDirectory.isEmpty() ? QDir::homePath() : m_previousDirectory;

    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Open File",
        initialPath,
        filter
    );

    if (!filePath.isEmpty()) {
        m_previousDirectory = QFileInfo(filePath).absolutePath();
        qDebug() << "File selected:" << filePath;
        callback(filePath);
    } else {
        qDebug() << "No file selected.";
    }
}

ObjectMgr::ControlPoints ObjectMgr::_loadBezierPointsOpenFile(const QString &path, bool *ok) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open file:" << path;
        showToast("Failed to open file");

        if (ok) {
            *ok = false;
        }
        return {};
    }

    ControlPoints controlPoints = _loadBezierPointsParse(file, ok);
    file.close();

    return controlPoints;
}

ObjectMgr::ControlPoints ObjectMgr::_loadBezierPointsParse(QFile &file, bool *ok) {
    ControlPoints controlPoints{};

    QTextStream istream(&file);
    size_t idx = 0;
    while (!istream.atEnd()) {
        QString line = istream.readLine();
        QStringList tokens = line.split(" ", Qt::SkipEmptyParts);

        if (tokens.size() != 3) {
            qWarning() << "Invalid line format, expected 3 values per line, got:" << tokens.size();

            if (!tokens.empty()) {
                qWarning() << "Wrong line content:" << line;

                if (ok) {
                    *ok = false;
                }

                showToast("Invalid file format");
                return controlPoints;
            }

            continue;
        }

        bool okX, okY, okZ;
        float x = tokens[0].toFloat(&okX);
        float y = tokens[1].toFloat(&okY);
        float z = tokens[2].toFloat(&okZ);

        if (okX && okY && okZ) {
            if (idx >= BEZIER_CONSTANTS::CONTROL_POINTS_COUNT) {
                qWarning() << "Too many control points, expected:" << BEZIER_CONSTANTS::CONTROL_POINTS_COUNT;

                if (ok) {
                    *ok = false;
                }

                showToast("Too many control points");
                return controlPoints;
            }

            controlPoints[idx++] = QVector3D(x, y, z);
        } else {
            qWarning() << "Failed to convert line to 3D point:" << line;

            if (ok) {
                *ok = false;
            }

            showToast("Invalid file format");
            return controlPoints;
        }
    }

    if (idx != BEZIER_CONSTANTS::CONTROL_POINTS_COUNT) {
        qWarning() << "Invalid number of control points, expected:" << BEZIER_CONSTANTS::CONTROL_POINTS_COUNT << "got:" << idx;

        if (ok) {
            *ok = false;
        }

        showToast("Wrong number of control points");
        return controlPoints;
    }

    if (ok) {
        *ok = true;
    }
    return controlPoints;
}

void ObjectMgr::showToast(const QString &message, int duration) {
    /* temporary */
    return;

    auto *toast = new QLabel(m_parentWidget);

    toast->setText(message);
    toast->setStyleSheet("background-color: rgba(211, 211, 211, 180); padding: 10px; border-radius: 10px;");
    toast->setAlignment(Qt::AlignCenter);

    toast->adjustSize();

    int padding = 20;
    toast->move(padding, m_parentWidget->height() - toast->height() - padding);

    toast->setWindowFlags(Qt::ToolTip);

    toast->show();

    QTimer::singleShot(duration, toast, &QLabel::deleteLater);
}

void ObjectMgr::_drawNet() {
    /* Draw control points */
    for (auto point: m_controlPoints) {
        m_drawingWidget->drawBezierPoint(rotate(point));
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
            auto point1 = m_controlPoints[i];
            auto point2 = m_controlPoints[idx];

            m_drawingWidget->drawBezierLine(rotate(point1), rotate(point2));
        }
    }

    /* Add triangle lines */
    for (const auto &triangle: m_triangles) {
        m_drawingWidget->drawTriangleLines(triangle[0].rotatedPosition, triangle[1].rotatedPosition);
        m_drawingWidget->drawTriangleLines(triangle[1].rotatedPosition, triangle[2].rotatedPosition);
        m_drawingWidget->drawTriangleLines(triangle[2].rotatedPosition, triangle[0].rotatedPosition);
    }
}

QVector3D &ObjectMgr::rotateZ(QVector3D &point, double angle) {
    if (angle == 0) {
        return point;
    }

    const double x = point.x();
    const double y = point.y();
    const double rad = angle * M_PI / 180.0;

    point.setX(static_cast<float>(x * std::cos(rad) - y * sin(rad)));
    point.setY(static_cast<float>(x * std::sin(rad) + y * cos(rad)));

    return point;
}

QVector3D &ObjectMgr::rotateX(QVector3D &point, double angle) {
    if (angle == 0) {
        return point;
    }

    const double y = point.y();
    const double z = point.z();
    const double rad = angle * M_PI / 180.0;

    point.setY(static_cast<float>(y * std::cos(rad) - z * sin(rad)));
    point.setZ(static_cast<float>(y * std::sin(rad) + z * cos(rad)));

    return point;
}

QVector3D &ObjectMgr::rotate(QVector3D &point) const {
    return rotateX(rotateZ(point, m_alpha), m_beta);
}

void ObjectMgr::_interpolateBezier() {
    m_triangles.clear();
    const float step = 1.0f / static_cast<float>(m_triangleAccuracy - 1);
    const int steps = m_triangleAccuracy;

    for (int i = 0; i < steps - 1; ++i) {
        for (int j = 0; j < steps - 1; ++j) {
            float u = static_cast<float>(i) * step;
            float v = static_cast<float>(j) * step;
            float u_next = static_cast<float>(i + 1) * step;
            float v_next = static_cast<float>(j + 1) * step;

            auto bu = _computeBernstein(u);
            auto bv = _computeBernstein(v);
            auto bu_next = _computeBernstein(u_next);
            auto bv_next = _computeBernstein(v_next);

            auto [p00, pu00, pv00] = _computePointAndDeriv(bu, bv);
            auto [p10, pu10, pv10] = _computePointAndDeriv(bu_next, bv);
            auto [p01, pu01, pv01] = _computePointAndDeriv(bu, bv_next);
            auto [p11, pu11, pv11] = _computePointAndDeriv(bu_next, bv_next);

            QVector3D n00 = QVector3D::crossProduct(pu00, pv00).normalized();
            QVector3D n10 = QVector3D::crossProduct(pu10, pv10).normalized();
            QVector3D n01 = QVector3D::crossProduct(pu01, pv01).normalized();
            QVector3D n11 = QVector3D::crossProduct(pu11, pv11).normalized();

            Triangle t1, t2;

            t1[0] = Vertex(p00, pu00, pv00, n00, u, v, m_alpha, m_beta);
            t1[1] = Vertex(p10, pu10, pv10, n10, u_next, v, m_alpha, m_beta);
            t1[2] = Vertex(p01, pu01, pv01, n01, u, v_next, m_alpha, m_beta);

            t2[0] = Vertex(p10, pu10, pv10, n10, u_next, v, m_alpha, m_beta);
            t2[1] = Vertex(p11, pu11, pv11, n11, u_next, v_next, m_alpha, m_beta);
            t2[2] = Vertex(p01, pu01, pv01, n01, u, v_next, m_alpha, m_beta);

            m_triangles.push_back(t1);
            m_triangles.push_back(t2);
        }
    }
}

ObjectMgr::BernsteinTable ObjectMgr::_computeBernstein(const float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    const float mt = 1.0f - t;
    const float mt2 = mt * mt;
    const float mt3 = mt2 * mt;
    return {
        mt3, // (1-t)^3
        3.0f * mt2 * t, // 3(1-t)^2t
        3.0f * mt * t2, // 3(1-t)t^2
        t3 // t^3
    };
}

std::tuple<QVector3D, QVector3D, QVector3D>
ObjectMgr::_computePointAndDeriv(const BernsteinTable &bu, const BernsteinTable &bv) const {
    QVector3D derivativeU{};
    QVector3D derivativeV{};
    QVector3D point{};
    static constexpr std::array<float, 4> derivativeCoeffs = {-3.0f, -6.0f, 3.0f, 6.0f};

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            derivativeV += m_controlPoints[i * 4 + j] * (derivativeCoeffs[i] * bv[j]);
            derivativeU += m_controlPoints[i * 4 + j] * (bu[i] * derivativeCoeffs[j]);
            point += m_controlPoints[i * 4 + j] * (bu[i] * bv[j]);
        }
    }
    return {point, derivativeU, derivativeV};
}

void ObjectMgr::_loadTexture(const QString &path) {
    auto pTexture = _loadTextureFromFile(path);

    if (!pTexture) {
        return;
    }

    delete m_texture;
    m_texture = pTexture;

    m_drawingWidget->setTexture(m_texture);

    if (m_useTexture) {
        m_drawingWidget->setFillType(FillType::TEXTURE);
    }
}

QImage *ObjectMgr::_loadTextureFromFile(const QString &path) {
    QImage image(path);

    if (image.isNull()) {
        qWarning() << "Failed to load image from path:" << path;
        showToast("Failed to load image");
        return nullptr;
    }

    return new QImage(
            image.scaled(RESOURCE_CONSTANTS::TEXTURE_IMAGE_SIZE, RESOURCE_CONSTANTS::TEXTURE_IMAGE_SIZE,
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
