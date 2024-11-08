//
// Created by Jlisowskyy on 11/5/24.
//

#ifndef OBJECTMGR_H
#define OBJECTMGR_H

/* internal includes */
#include "../Constants.h"
#include "../PrimitiveData/Triangle.h"

/* external includes */
#include <QObject>
#include <QColor>
#include <QWidget>
#include <array>
#include <QVector3D>
#include <QFile>

/* Forward declarations */
class ToolBar;

class DrawingWidget;

class Mesh;

class ObjectMgr : public QObject {
    Q_OBJECT

    // ------------------------------
    // Class creation
    // ------------------------------
public:
    explicit ObjectMgr(QObject *parent, QWidget *widgetParent, DrawingWidget *drawingWidget);

    ~ObjectMgr() override;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void connectToToolBar(ToolBar *toolBar);

    void loadDefaultSettings();

    void redraw();

    // ------------------------------
    // Class slots
    // ------------------------------
public slots:
    /* State changes */

    void onTriangulationChanged(double value);

    void onAlphaChanged(double value);

    void onBetaChanged(double value);

    void onKSChanged(double value);

    void onKDChanged(double value);

    void onMChanged(double value);

    /* toggle actions */

    void onDrawNetChanged(bool isChecked);

    void onEnableTextureChanged(bool isChecked);

    void onEnableNormalVectorsChanged(bool isChecked);

    void onStopLightingMovementChanged(bool isChecked);

    /* simple actions */

    void onLoadBezierPointsTriggered();

    void onLoadTexturesTriggered();

    void onLoadNormalVectorsTriggered();

    void onColorChangedTriggered();

    void onLightColorChangedTriggered();

    // ------------------------------
    // Class protected methods
    // ------------------------------
protected:
    void _loadBezierPoints(const QString &path);

    void _openFileDialog(const std::function<void(const QString &)> &callback, const char* filter);

    [[nodiscard]] ControlPoints _loadBezierPointsOpenFile(const QString &path, bool *ok);

    [[nodiscard]] ControlPoints _loadBezierPointsParse(QFile &file, bool *ok);

    void _loadTexture(const QString &path);

    [[nodiscard]] QImage *_loadTextureFromFile(const QString &path);

    void _showToast(const QString &message, int duration = UI_CONSTANTS::DEFAULT_TOAST_DURATION_MS);

    void _drawNet();

    // ------------------------------
    // Class fields
    // ------------------------------
protected:
    QWidget *m_parentWidget{};
    DrawingWidget *m_drawingWidget{};
    QString m_previousDirectory{};
    Mesh *m_mesh{};

    bool m_drawNet{};
    bool m_useTexture{};
    QImage *m_texture{};
};


#endif //OBJECTMGR_H
