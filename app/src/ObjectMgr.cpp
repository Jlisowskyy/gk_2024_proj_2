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

ObjectMgr::ObjectMgr(QObject *parent, QWidget *widgetParent, DrawingWidget *drawingWidget) :
        QObject(parent),
        m_parentWidget(widgetParent),
        m_drawingWidget(drawingWidget) {
    Q_ASSERT(parent && widgetParent && drawingWidget);
}

ObjectMgr::~ObjectMgr() = default;

void ObjectMgr::connectToToolBar(ToolBar *toolBar) {
    /* State changes */
    std::vector<std::pair<DoubleSlider *, void (ObjectMgr::*)(double)> > vSliderProc{
            {toolBar->m_triangulationSlider, &ObjectMgr::onTriangulationChanged},
            {toolBar->m_alphaSlider,         &ObjectMgr::onAlphaChanged},
            {toolBar->m_betaSlider,          &ObjectMgr::onBetaChanged},
            {toolBar->m_ksSlider,            &ObjectMgr::onKSChanged},
            {toolBar->m_kdSlider,            &ObjectMgr::onKDChanged},
            {toolBar->m_mSlider,             &ObjectMgr::onMChanged},
    };

    for (const auto &[slider, proc]: vSliderProc) {
        connect(slider, &DoubleSlider::doubleValueChanged, this, proc);
    }

    /* simple actions */
    std::vector<std::pair<QAction *, void (ObjectMgr::*)()> > vActionProc{
            {toolBar->m_loadTextureButton,       &ObjectMgr::onLoadTexturesTriggered},
            {toolBar->m_loadBezierPointsButton,  &ObjectMgr::onLoadBezierPointsTriggered},
            {toolBar->m_loadNormalVectorsButton, &ObjectMgr::onLoadNormalVectorsTriggered},
            {toolBar->m_changePlainColorButton,  &ObjectMgr::onColorChangedTriggered},
    };

    for (const auto &[action, proc]: vActionProc) {
        connect(action, &QAction::triggered, this, proc);
    }

    /* Check able actions */
    std::vector<std::pair<QAction *, void (ObjectMgr::*)(bool)> > vActionBoolProc{
            {toolBar->m_drawNetButton,             &ObjectMgr::onDrawNetChanged},
            {toolBar->m_enableTextureButton,       &ObjectMgr::onEnableTextureChanged},
            {toolBar->m_enableNormalVectorsButton, &ObjectMgr::onEnableNormalVectorsChanged},
            {toolBar->m_stopLightMovementButton,   &ObjectMgr::onStopLightingMovementChanged},
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
    m_color = DEFAULT_PLAIN_COLOR;
    m_drawNet = true;
    m_triangleAccuracy = DEFAULT_TRIANGLE_ACCURACY;
    m_alpha = 0;
    m_beta = 0;

    _loadBezierPoints(DEFAULT_DATA_PATH);
    redraw();
}

void ObjectMgr::redraw() {
    m_drawingWidget->clearContent();
    m_triangles.clear();

    _interpolateBezier();
    m_drawingWidget->setTriangles(&m_triangles);

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

void ObjectMgr::onDrawNetChanged(bool isChecked) {
    m_drawNet = isChecked;
    redraw();
}

void ObjectMgr::onEnableTextureChanged(bool isChecked) {
}

void ObjectMgr::onEnableNormalVectorsChanged(bool isChecked) {
}

void ObjectMgr::onStopLightingMovementChanged(bool isChecked) {
}

void ObjectMgr::onLoadBezierPointsTriggered() {
    _openFileDialog([this](const QString &path) {
        _loadBezierPoints(path);
    });
}

void ObjectMgr::onLoadTexturesTriggered() {
    _openFileDialog([](const QString &path) {
        qDebug() << "Loading texture from path:" << path;
    });
}

void ObjectMgr::onLoadNormalVectorsTriggered() {
    _openFileDialog([](const QString &path) {
        qDebug() << "Loading normal vectors from path:" << path;
    });
}

void ObjectMgr::onColorChangedTriggered() {
    const QColor selectedColor = QColorDialog::getColor(Qt::white, m_parentWidget, "Choose Color");
    if (!selectedColor.isValid()) {
        qDebug() << "Error getting color";
        return;
    }

    m_color = selectedColor;
    redraw();
}

void ObjectMgr::_loadBezierPoints(const QString &path) {
    bool ok;
    ControlPoints controlPoints = _loadBezierPointsOpenFile(path, &ok);
    if (!ok) {
        qWarning() << "Failed to load bezier points from file:" << path;
        return;
    }

    m_controlPoints = controlPoints;
    redraw();
}

void ObjectMgr::_openFileDialog(std::function<void(const QString &)> callback) {
    Q_ASSERT(callback);

    QString initialPath = m_previousDirectory.isEmpty() ? QDir::homePath() : m_previousDirectory;

    QString filePath = QFileDialog::getOpenFileName(
            nullptr,
            "Open File",
            initialPath,
            "Text Files (*.txt);;All Files (*)"
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
            if (idx >= CONTROL_POINTS_COUNT) {
                qWarning() << "Too many control points, expected:" << CONTROL_POINTS_COUNT;

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

    if (idx != CONTROL_POINTS_COUNT) {
        qWarning() << "Invalid number of control points, expected:" << CONTROL_POINTS_COUNT << "got:" << idx;

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
    static constexpr int CONTROL_POINTS_MATRIX_SIZE_INT = CONTROL_POINTS_MATRIX_SIZE;
    static constexpr int CONTROL_POINTS_COUNT_INT = CONTROL_POINTS_COUNT;
    for (int i = 0; i < CONTROL_POINTS_COUNT_INT - 1; i++) {
        const int row = i / CONTROL_POINTS_MATRIX_SIZE_INT;
        const int col = i % CONTROL_POINTS_MATRIX_SIZE_INT;

        const int dx[] = {0, 1};
        const int dy[] = {1, 0};

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
        m_drawingWidget->drawTriangleLines(triangle.v1.rotatedPosition, triangle.v2.rotatedPosition);
        m_drawingWidget->drawTriangleLines(triangle.v2.rotatedPosition, triangle.v3.rotatedPosition);
        m_drawingWidget->drawTriangleLines(triangle.v3.rotatedPosition, triangle.v1.rotatedPosition);
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

    auto bernstein = [](float t) -> std::array<float, 4> {
        const float t2 = t * t;
        const float t3 = t2 * t;
        const float mt = 1.0f - t;
        const float mt2 = mt * mt;
        const float mt3 = mt2 * mt;
        return {
                mt3,                    // (1-t)³
                3.0f * mt2 * t,        // 3(1-t)²t
                3.0f * mt * t2,        // 3(1-t)t²
                t3                      // t³
        };
    };

    auto computePoint = [this](const std::array<float, 4> &bu, const std::array<float, 4> &bv) -> QVector3D {
        QVector3D point(0, 0, 0);
        for (int k = 0; k < 4; ++k) {
            for (int l = 0; l < 4; ++l) {
                point += m_controlPoints[k * 4 + l] * (bu[k] * bv[l]);
            }
        }
        return point;
    };

    for (int i = 0; i < steps - 1; ++i) {
        for (int j = 0; j < steps - 1; ++j) {
            float u = static_cast<float>(i) * step;
            float v = static_cast<float>(j) * step;
            float u_next = static_cast<float>(i + 1) * step;
            float v_next = static_cast<float>(j + 1) * step;

            auto bu = bernstein(u);
            auto bv = bernstein(v);
            auto bu_next = bernstein(u_next);
            auto bv_next = bernstein(v_next);

            QVector3D p00 = computePoint(bu, bv);
            QVector3D p10 = computePoint(bu_next, bv);
            QVector3D p01 = computePoint(bu, bv_next);
            QVector3D p11 = computePoint(bu_next, bv_next);

            Traingle t1, t2;

            // First triangle
            t1.v1.position = p00;
            t1.v1.u = u;
            t1.v1.v = v;

            t1.v2.position = p10;
            t1.v2.u = u_next;
            t1.v2.v = v;

            t1.v3.position = p01;
            t1.v3.u = u;
            t1.v3.v = v_next;

            // Second triangle
            t2.v1.position = p10;
            t2.v1.u = u_next;
            t2.v1.v = v;

            t2.v2.position = p11;
            t2.v2.u = u_next;
            t2.v2.v = v_next;

            t2.v3.position = p01;
            t2.v3.u = u;
            t2.v3.v = v_next;

            // Apply rotation to position vectors
            t1.v1.rotatedPosition = t1.v1.position;
            t1.v2.rotatedPosition = t1.v2.position;
            t1.v3.rotatedPosition = t1.v3.position;
            rotate(t1.v1.rotatedPosition);
            rotate(t1.v2.rotatedPosition);
            rotate(t1.v3.rotatedPosition);

            t2.v1.rotatedPosition = t2.v1.position;
            t2.v2.rotatedPosition = t2.v2.position;
            t2.v3.rotatedPosition = t2.v3.position;
            rotate(t2.v1.rotatedPosition);
            rotate(t2.v2.rotatedPosition);
            rotate(t2.v3.rotatedPosition);

            // Add triangles to the mesh
            m_triangles.push_back(t1);
            m_triangles.push_back(t2);
        }
    }
}