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

class ObjectMgr : public QObject {
Q_OBJECT

    // ------------------------------
    // Class creation
    // ------------------------------
public:
    explicit ObjectMgr(QObject *parent, QWidget *widgetParent);

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

    static void openFileDialog(std::function<void(const QString &)> callback);

    static ControlPoints loadBezierPoints(const QString &path, bool *ok);

    static ControlPoints loadBezierPointsParse(QFile &file, bool *ok);

    // ------------------------------
    // Class fields
    // ------------------------------
protected:

    QColor m_color;
    QWidget *m_parentWidget;

    ControlPoints m_controlPoints;
};


#endif //OBJECTMGR_H
