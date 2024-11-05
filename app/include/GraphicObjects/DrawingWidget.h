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
#include <QVector3D>

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

    void drawBezierPoint(QVector3D point);

    void drawBezierLine(QVector3D start, QVector3D end);

    [[nodiscard]] QPointF dropPointToScreen(const QVector3D &point) const;

    // ------------------------------
    // Class slots
    // ------------------------------
public slots:

    void clearContent();

    void setObserverDistance(double distance);

    // ------------------------------
    // Class signals
    // ------------------------------
signals:


    // ------------------------------
    // Protected methods
    // ------------------------------
protected:

    void resizeEvent(QResizeEvent *event) override;

    // ------------------------------
    // Private methods
    // ------------------------------

private:

    void updateScene();

    void updateElements();

    void _drawBezierPoint(const QVector3D &point, size_t idx);
    void _drawBezierLine(const std::pair<QVector3D, QVector3D> &line);

    // ------------------------------
    // Class fields
    // ------------------------------

    QGraphicsScene *m_scene{};
    double m_observerDistance{};

    std::vector<QVector3D> m_points{};
    std::vector<std::pair<QVector3D, QVector3D>> m_lines{};
};


#endif //APP_DRAWINGWIDGET_H
