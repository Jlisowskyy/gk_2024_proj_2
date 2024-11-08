//
// Created by Jlisowskyy on 11/5/24.
//

/* internal includes */
#include "../include/ManagingObjects/ObjectMgr.h"
#include "../include/ManagingObjects/ToolBar.h"
#include "../include/UiObjects/DoubleSlider.h"
#include "../include/GraphicObjects/DrawingWidget.h"
#include "../include/Rendering/Mesh.h"

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
        {toolBar->m_changeLightColorButton, &ObjectMgr::onLightColorChangedTriggered},
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
    m_useTexture = false;

    m_drawingWidget->setKdCoef(LIGHTING_CONSTANTS::DEFAULT_KD);
    m_drawingWidget->setKsCoef(LIGHTING_CONSTANTS::DEFAULT_KS);
    m_drawingWidget->setMCoef(LIGHTING_CONSTANTS::DEFAULT_M);
    m_drawingWidget->setObserverDistance(VIEW_SETTINGS::DEFAULT_OBSERVER_DISTANCE);
    m_drawingWidget->setLightZ(VIEW_SETTINGS::DEFAULT_LIGHT_Z);
    m_drawingWidget->setColor(UI_CONSTANTS::DEFAULT_PLAIN_COLOR);

    m_drawingWidget->setKdCoef(LIGHTING_CONSTANTS::DEFAULT_KD);
    m_drawingWidget->setKsCoef(LIGHTING_CONSTANTS::DEFAULT_KS);
    m_drawingWidget->setMCoef(LIGHTING_CONSTANTS::DEFAULT_M);
    m_drawingWidget->setLightColor(LIGHTING_CONSTANTS::DEFAULT_LIGHT_COLOR);
    m_drawingWidget->setLightZ(VIEW_SETTINGS::DEFAULT_LIGHT_Z);

    _loadTexture(RESOURCE_CONSTANTS::DEFAULT_TEXTURE_PATH);

    m_mesh = new Mesh(_loadBezierPointsOpenFile(RESOURCE_CONSTANTS::DEFAULT_CONTROL_POINTS_PATH, nullptr),
                      VIEW_SETTINGS::DEFAULT_ALPHA, VIEW_SETTINGS::DEFAULT_BETA,
                      VIEW_SETTINGS::DEFAULT_TRIANGLE_ACCURACY);

    redraw();
}

void ObjectMgr::redraw() {
    m_drawingWidget->clearContent();
    m_triangles.clear();

    if (m_drawNet) {
        _drawNet();
    }
    m_drawingWidget->updateScene();
}

void ObjectMgr::onTriangulationChanged(const double value) {
    m_mesh->setAccuracy(value);
    redraw();
}

void ObjectMgr::onAlphaChanged(const double value) {
    m_mesh->setAlpha(value);
    redraw();
}

void ObjectMgr::onBetaChanged(const double value) {
    m_mesh->setBeta(value);
    redraw();
}

void ObjectMgr::onKSChanged(const double value) {
    m_drawingWidget->setKsCoef(static_cast<float>(value));
}

void ObjectMgr::onKDChanged(const double value) {
    m_drawingWidget->setKdCoef(static_cast<float>(value));
}

void ObjectMgr::onMChanged(const double value) {
    m_drawingWidget->setMCoef(static_cast<float>(value));
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

void ObjectMgr::onStopLightingMovementChanged(const bool isChecked) {
    m_drawingWidget->setStopLight(isChecked);
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
}

void ObjectMgr::_loadBezierPoints(const QString &path) {
    bool ok;
    const ControlPoints controlPoints = _loadBezierPointsOpenFile(path, &ok);
    if (!ok) {
        qWarning() << "Failed to load bezier points from file:" << path;
        return;
    }

    m_mesh->setControlPoints(controlPoints);
    redraw();
}

void ObjectMgr::_openFileDialog(const std::function<void(const QString &)> &callback, const char *filter) {
    Q_ASSERT(callback);

    const QString initialPath = m_previousDirectory.isEmpty() ? QDir::homePath() : m_previousDirectory;
    const QString filePath = QFileDialog::getOpenFileName(
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

ControlPoints ObjectMgr::_loadBezierPointsOpenFile(const QString &path, bool *ok) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open file:" << path;
        _showToast("Failed to open file");

        if (ok) {
            *ok = false;
        }
        return {};
    }

    const ControlPoints controlPoints = _loadBezierPointsParse(file, ok);
    file.close();

    return controlPoints;
}

ControlPoints ObjectMgr::_loadBezierPointsParse(QFile &file, bool *ok) {
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

                _showToast("Invalid file format");
                return controlPoints;
            }

            continue;
        }

        bool okX, okY, okZ;
        const auto x = tokens[0].toFloat(&okX);
        const auto y = tokens[1].toFloat(&okY);
        const auto z = tokens[2].toFloat(&okZ);

        if (okX && okY && okZ) {
            if (idx >= BEZIER_CONSTANTS::CONTROL_POINTS_COUNT) {
                qWarning() << "Too many control points, expected:" << BEZIER_CONSTANTS::CONTROL_POINTS_COUNT;

                if (ok) {
                    *ok = false;
                }

                _showToast("Too many control points");
                return controlPoints;
            }

            controlPoints[idx++] = QVector3D(x, y, z);
        } else {
            qWarning() << "Failed to convert line to 3D point:" << line;

            if (ok) {
                *ok = false;
            }

            _showToast("Invalid file format");
            return controlPoints;
        }
    }

    if (idx != BEZIER_CONSTANTS::CONTROL_POINTS_COUNT) {
        qWarning() << "Invalid number of control points, expected:" << BEZIER_CONSTANTS::CONTROL_POINTS_COUNT << "got:"
                << idx;

        if (ok) {
            *ok = false;
        }

        _showToast("Wrong number of control points");
        return controlPoints;
    }

    if (ok) {
        *ok = true;
    }
    return controlPoints;
}

void ObjectMgr::_showToast(const QString &message, int duration) {
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
    for (auto point: m_mesh->getControlPoints()) {
        m_drawingWidget->drawBezierPoint(m_mesh->getPointAlignedWithMeshPlain(point));
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
            auto point1 = m_mesh->getControlPoints()[i];
            auto point2 = m_mesh->getControlPoints()[idx];

            m_drawingWidget->drawBezierLine(m_mesh->getPointAlignedWithMeshPlain(point1),
                                            m_mesh->getPointAlignedWithMeshPlain(point2));
        }
    }

    /* Add triangle lines */
    for (const auto &triangle: m_triangles) {
        m_drawingWidget->drawTriangleLines(triangle[0].rotatedPosition, triangle[1].rotatedPosition);
        m_drawingWidget->drawTriangleLines(triangle[1].rotatedPosition, triangle[2].rotatedPosition);
        m_drawingWidget->drawTriangleLines(triangle[2].rotatedPosition, triangle[0].rotatedPosition);
    }
}

void ObjectMgr::_loadTexture(const QString &path) {
    const auto pTexture = _loadTextureFromFile(path);

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
    const QImage image(path);

    if (image.isNull()) {
        qWarning() << "Failed to load image from path:" << path;
        _showToast("Failed to load image");
        return nullptr;
    }

    return new QImage(
        image.scaled(RESOURCE_CONSTANTS::TEXTURE_IMAGE_SIZE, RESOURCE_CONSTANTS::TEXTURE_IMAGE_SIZE,
                     Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ObjectMgr::onLightColorChangedTriggered() {
    const QColor selectedColor = QColorDialog::getColor(Qt::white, m_parentWidget, "Choose Color");
    if (!selectedColor.isValid()) {
        qDebug() << "Error getting color";
        return;
    }

    m_drawingWidget->setLightColor(selectedColor);
}
