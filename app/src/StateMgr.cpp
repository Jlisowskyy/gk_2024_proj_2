//
// Created by Jlisowskyy on 11/5/24.
//

/* internal includes */
#include "../include/ManagingObjects/StateMgr.h"
#include "../include/UiObjects/ToolBar.h"
#include "../include/UiObjects/DoubleSlider.h"
#include "../include/GraphicObjects/DrawingWidget.h"
#include "../include/Rendering/Mesh.h"
#include "../include/Rendering/Texture.h"
#include "../include/ManagingObjects/SceneMgr.h"

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

StateMgr::StateMgr(QObject *parent, QWidget *widgetParent, DrawingWidget *drawingWidget) : QObject(parent),
    m_parentWidget(widgetParent),
    m_drawingWidget(drawingWidget) {
    Q_ASSERT(parent && widgetParent && drawingWidget);
}

StateMgr::~StateMgr() = default;

void StateMgr::connectToToolBar(ToolBar *toolBar) {
    /* State changes */
    std::vector<std::pair<DoubleSlider *, void (StateMgr::*)(double)> > vSliderProc{
        {toolBar->m_triangulationSlider, &StateMgr::onTriangulationChanged},
        {toolBar->m_alphaSlider, &StateMgr::onAlphaChanged},
        {toolBar->m_betaSlider, &StateMgr::onBetaChanged},
        {toolBar->m_ksSlider, &StateMgr::onKSChanged},
        {toolBar->m_kdSlider, &StateMgr::onKDChanged},
        {toolBar->m_mSlider, &StateMgr::onMChanged},
    };

    for (const auto &[slider, proc]: vSliderProc) {
        connect(slider, &DoubleSlider::doubleValueChanged, this, proc);
    }

    /* simple actions */
    std::vector<std::pair<QAction *, void (StateMgr::*)()> > vActionProc{
        {toolBar->m_loadTextureButton, &StateMgr::onLoadTexturesTriggered},
        {toolBar->m_loadBezierPointsButton, &StateMgr::onLoadBezierPointsTriggered},
        {toolBar->m_loadNormalVectorsButton, &StateMgr::onLoadNormalVectorsTriggered},
        {toolBar->m_changePlainColorButton, &StateMgr::onColorChangedTriggered},
        {toolBar->m_changeLightColorButton, &StateMgr::onLightColorChangedTriggered},
    };

    for (const auto &[action, proc]: vActionProc) {
        connect(action, &QAction::triggered, this, proc);
    }

    /* Check able actions */
    std::vector<std::pair<QAction *, void (StateMgr::*)(bool)> > vActionBoolProc{
        {toolBar->m_drawNetButton, &StateMgr::onDrawNetChanged},
        {toolBar->m_enableTextureButton, &StateMgr::onEnableTextureChanged},
        {toolBar->m_enableNormalVectorsButton, &StateMgr::onEnableNormalVectorsChanged},
        {toolBar->m_stopLightMovementButton, &StateMgr::onStopLightingMovementChanged},
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

void StateMgr::loadDefaultSettings() {
    m_sceneMgr = new SceneMgr(this, UI_CONSTANTS::DEFAULT_PLAIN_COLOR,
                              UI_CONSTANTS::DEFAULT_DRAW_NET,
                              UI_CONSTANTS::DEFAULT_USE_TEXTURE,
                              UI_CONSTANTS::DEFAULT_PLAY_ANIMATION,
                              _loadTextureFromFile(RESOURCE_CONSTANTS::DEFAULT_TEXTURE_PATH));

    m_drawingWidget->setObserverDistance(VIEW_SETTINGS::DEFAULT_OBSERVER_DISTANCE);
    m_drawingWidget->setLightZ(VIEW_SETTINGS::DEFAULT_LIGHT_Z);
    m_drawingWidget->setLightColor(LIGHTING_CONSTANTS::DEFAULT_LIGHT_COLOR);
    m_drawingWidget->setLightZ(VIEW_SETTINGS::DEFAULT_LIGHT_Z);

    _loadTexture(RESOURCE_CONSTANTS::DEFAULT_TEXTURE_PATH);

    m_mesh = new Mesh(_loadBezierPointsOpenFile(RESOURCE_CONSTANTS::DEFAULT_CONTROL_POINTS_PATH, nullptr),
                      VIEW_SETTINGS::DEFAULT_ALPHA, VIEW_SETTINGS::DEFAULT_BETA,
                      VIEW_SETTINGS::DEFAULT_TRIANGLE_ACCURACY);

    m_texture = new Texture(LIGHTING_CONSTANTS::DEFAULT_KS, LIGHTING_CONSTANTS::DEFAULT_KD,
                            LIGHTING_CONSTANTS::DEFAULT_M, LIGHTING_CONSTANTS::DEFAULT_LIGHT_COLOR);

    redraw();
}

void StateMgr::redraw() {
    m_sceneMgr->redrawScene(*m_drawingWidget, *m_texture, *m_mesh);
}

void StateMgr::onTriangulationChanged(const double value) {
    m_mesh->setAccuracy(value);
    redraw();
}

void StateMgr::onAlphaChanged(const double value) {
    m_mesh->setAlpha(value);
    redraw();
}

void StateMgr::onBetaChanged(const double value) {
    m_mesh->setBeta(value);
    redraw();
}

void StateMgr::onKSChanged(const double value) {
    m_texture->setKsCoef(static_cast<float>(value));
}

void StateMgr::onKDChanged(const double value) {
    m_texture->setKdCoef(static_cast<float>(value));
}

void StateMgr::onMChanged(const double value) {
    m_texture->setMCoef(static_cast<float>(value));
}

void StateMgr::onDrawNetChanged(const bool isChecked) {
    m_sceneMgr->setDrawNet(isChecked);
    redraw();
}

void StateMgr::onEnableTextureChanged(const bool isChecked) {
    m_sceneMgr->setUseTexture(isChecked);
}

void StateMgr::onEnableNormalVectorsChanged(bool isChecked) {
}

void StateMgr::onStopLightingMovementChanged(const bool isChecked) {
    m_drawingWidget->setStopLight(isChecked);
}

void StateMgr::onLoadBezierPointsTriggered() {
    _openFileDialog([this](const QString &path) {
                        _loadBezierPoints(path);
                    },
                    "Text Files (*.txt);;All Files (*)");
}

void StateMgr::onLoadTexturesTriggered() {
    _openFileDialog([this](const QString &path) {
        _loadTexture(path);
    }, "Images (*.png *.jpg *.bmp);;All Files (*)");
}

void StateMgr::onLoadNormalVectorsTriggered() {
    _openFileDialog([](const QString &path) {
        qDebug() << "Loading normal vectors from path:" << path;
    }, "");
}

void StateMgr::onColorChangedTriggered() {
    const QColor selectedColor = QColorDialog::getColor(Qt::white, m_parentWidget, "Choose Color");
    if (!selectedColor.isValid()) {
        qDebug() << "Error getting color";
        return;
    }

    m_sceneMgr->setColor(selectedColor);
}

void StateMgr::_loadBezierPoints(const QString &path) {
    bool ok;
    const ControlPoints controlPoints = _loadBezierPointsOpenFile(path, &ok);
    if (!ok) {
        qWarning() << "Failed to load bezier points from file:" << path;
        return;
    }

    m_mesh->setControlPoints(controlPoints);
    redraw();
}

void StateMgr::_openFileDialog(const std::function<void(const QString &)> &callback, const char *filter) {
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

ControlPoints StateMgr::_loadBezierPointsOpenFile(const QString &path, bool *ok) {
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

ControlPoints StateMgr::_loadBezierPointsParse(QFile &file, bool *ok) {
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

void StateMgr::_showToast(const QString &message, int duration) {
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

void StateMgr::_loadTexture(const QString &path) {
    const auto pTexture = _loadTextureFromFile(path);

    if (!pTexture) {
        return;
    }

    m_sceneMgr->setTextureImg(pTexture);
}

QImage *StateMgr::_loadTextureFromFile(const QString &path) {
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

void StateMgr::onLightColorChangedTriggered() {
    const QColor selectedColor = QColorDialog::getColor(Qt::white, m_parentWidget, "Choose Color");
    if (!selectedColor.isValid()) {
        qDebug() << "Error getting color";
        return;
    }

    m_drawingWidget->setLightColor(selectedColor);
}
