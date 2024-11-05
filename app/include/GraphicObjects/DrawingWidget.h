//
// Created by Jlisowskyy on 11/04/24.
//

#ifndef APP_DRAWINGWIDGET_H
#define APP_DRAWINGWIDGET_H

/* external includes */
#include <QObject>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QLabel>
#include <QGraphicsItem>

/* internal includes */
#include "../Constants.h"

/* Forward declaration */

class DrawingWidget : public QGraphicsView {
Q_OBJECT

    // ------------------------------
    // Class creation
    // ------------------------------
public:

    explicit DrawingWidget(QWidget *parent);

    ~DrawingWidget() override;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void drawPoint(QVector3D point, Qt::GlobalColor color, int radius);

    void drawLine(QVector3D start, QVector3D end, Qt::GlobalColor color, int width);

    // ------------------------------
    // Class slots
    // ------------------------------
public slots:

    void clearContent() const;

    void setObserverDistance(double distance) { m_observerDistance = distance; }

    // ------------------------------
    // Class signals
    // ------------------------------
signals:


    // ------------------------------
    // Protected methods
    // ------------------------------
protected:


    // ------------------------------
    // Private methods
    // ------------------------------

private:
    // ------------------------------
    // Class fields
    // ------------------------------

    QGraphicsScene *m_scene;
    double m_observerDistance;
};


#endif //APP_DRAWINGWIDGET_H
