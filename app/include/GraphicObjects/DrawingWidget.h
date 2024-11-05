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
#include <vector>

/* internal includes */
#include "../Constants.h"
#include "../PrimitiveData/Triangle.h"

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

    void drawBezierPoint(const QVector3D &point);

    void drawBezierLine(const QVector3D &start, const QVector3D &end);

    void drawTriangleLines(const QVector3D &start, const QVector3D &end);

    [[nodiscard]] QPointF dropPointToScreen(const QVector3D &point) const;

    void setTriangles(std::vector<Traingle> *triangles);

    // ------------------------------
    // Class slots
    // ------------------------------
public slots:

    void updateScene();

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


    void updateElements();

    void _drawBezierPoint(const QVector3D &point, size_t idx);

    void _drawBezierLine(const std::pair<QVector3D, QVector3D> &line);

    void _drawTriangleLine(const std::pair<QVector3D, QVector3D> &line);

    // ------------------------------
    // Class fields
    // ------------------------------

    QGraphicsScene *m_scene{};
    double m_observerDistance{};

    std::vector<QVector3D> m_points{};
    std::vector<std::pair<QVector3D, QVector3D>> m_lines{};
    std::vector<std::pair<QVector3D, QVector3D>> m_triangleLines{};
    std::vector<Traingle> *m_triangles{};
};


#endif //APP_DRAWINGWIDGET_H
