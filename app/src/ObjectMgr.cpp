//
// Created by Jlisowskyy on 11/5/24.
//

/* internal includes */
#include "../include/ManagingObjects/ObjectMgr.h"
#include "../include/ManagingObjects/ToolBar.h"
#include "../include/UiObjects/DoubleSlider.h"

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

ObjectMgr::ObjectMgr(QObject *parent, QWidget *widgetParent) : QObject(parent), m_parentWidget(widgetParent) {
    Q_ASSERT(parent && widgetParent);
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
}

void ObjectMgr::loadDefaultSettings() {
    m_color = DEFAULT_PLAIN_COLOR;
}

void ObjectMgr::redraw() {
}

void ObjectMgr::onTriangulationChanged(double value) {
}

void ObjectMgr::onAlphaChanged(double value) {
}

void ObjectMgr::onBetaChanged(double value) {
}

void ObjectMgr::onKSChanged(double value) {
}

void ObjectMgr::onKDChanged(double value) {
}

void ObjectMgr::onMChanged(double value) {
}

void ObjectMgr::onDrawNetChanged(bool isChecked) {
}

void ObjectMgr::onEnableTextureChanged(bool isChecked) {
}

void ObjectMgr::onEnableNormalVectorsChanged(bool isChecked) {
}

void ObjectMgr::onStopLightingMovementChanged(bool isChecked) {
}

void ObjectMgr::onLoadBezierPointsTriggered() {
    openFileDialog([this](const QString &path) {
        _loadBezierPoints(path);
    });
}

void ObjectMgr::onLoadTexturesTriggered() {
    openFileDialog([](const QString &path) {
        qDebug() << "Loading texture from path:" << path;
    });
}

void ObjectMgr::onLoadNormalVectorsTriggered() {
    openFileDialog([](const QString &path) {
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
    ControlPoints controlPoints = loadBezierPoints(path, &ok);
    if (!ok) {
        qWarning() << "Failed to load bezier points from file:" << path;
        return;
    }

    qDebug() << "Bezier points loaded successfully from file:" << path;
    redraw();
}

void ObjectMgr::openFileDialog(std::function<void(const QString &)> callback) {
    Q_ASSERT(callback);

    QString filePath = QFileDialog::getOpenFileName(
            nullptr,
            "Open File",
            QDir::homePath(),
            "Text Files (*.txt);;All Files (*)"
    );

    if (!filePath.isEmpty()) {
        qDebug() << "File selected:" << filePath;
        callback(filePath);
    } else {
        qDebug() << "No file selected.";
    }
}

ObjectMgr::ControlPoints ObjectMgr::loadBezierPoints(const QString &path, bool *ok) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open file:" << path;

        if (ok) {
            *ok = false;
        }
        return {};
    }

    ControlPoints controlPoints = loadBezierPointsParse(file, ok);
    file.close();

    return controlPoints;
}

ObjectMgr::ControlPoints ObjectMgr::loadBezierPointsParse(QFile &file, bool *ok) {
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
                break;
            }

            controlPoints[idx++] = QVector3D(x, y, z);
        } else {
            qWarning() << "Failed to convert line to 3D point:" << line;

            if (ok) {
                *ok = false;
            }

            return controlPoints;
        }
    }

    if (idx != CONTROL_POINTS_COUNT) {
        qWarning() << "Invalid number of control points, expected:" << CONTROL_POINTS_COUNT << "got:" << idx;

        if (ok) {
            *ok = false;
        }

        return controlPoints;
    }

    if (ok) {
        *ok = true;
    }
    return controlPoints;
}
