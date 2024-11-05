//
// Created by Jlisowskyy on 11/5/24.
//

#ifndef OBJECTMGR_H
#define OBJECTMGR_H

/* internal includes */
#include "../Constants.h"

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

class ObjectMgr : public QObject {
Q_OBJECT

    // ------------------------------
    // Class creation
    // ------------------------------
public:
    explicit ObjectMgr(QObject *parent, QWidget *widgetParent, DrawingWidget *drawingWidget);

    ~ObjectMgr() override;

    // ------------------------------
    // Class types
    // ------------------------------

    using ControlPoints = std::array<QVector3D, CONTROL_POINTS_COUNT>;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void connectToToolBar(ToolBar *toolBar);

    void loadDefaultSettings();

    void redraw();

    static QVector3D & rotateZ(QVector3D &point, double angle);

    static QVector3D & rotateX(QVector3D &point, double angle);

    QVector3D & rotate(QVector3D &point) const;

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

    // ------------------------------
    // Class protected methods
    // ------------------------------
protected:

    void _loadBezierPoints(const QString &path);

    void _openFileDialog(std::function<void(const QString &)> callback);

    ControlPoints _loadBezierPointsOpenFile(const QString &path, bool *ok);

    ControlPoints _loadBezierPointsParse(QFile &file, bool *ok);

    void showToast(const QString &message, int duration = DEFAULT_TOAST_DURATION_MS);

    void _drawNet();

    // ------------------------------
    // Class fields
    // ------------------------------
protected:

    QColor m_color{};
    QWidget *m_parentWidget{};
    DrawingWidget *m_drawingWidget{};

    ControlPoints m_controlPoints{};

    QString m_previousDirectory{};

    bool m_drawNet{};

    double m_alpha{};
    double m_beta{};
};


#endif //OBJECTMGR_H
