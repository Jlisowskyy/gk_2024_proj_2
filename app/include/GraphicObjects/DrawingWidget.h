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
#include <QPixmap>
#include <array>
#include <list>
#include <QTimer>
#include <QGraphicsEllipseItem>

/* internal includes */
#include "../include/Intf.h"

/* Forward declaration */
class StateMgr;

class DrawingWidget : public QGraphicsView {
    Q_OBJECT

    // ------------------------------
    // Class defs
    // ------------------------------
public:
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit DrawingWidget(QWidget *parent);

    ~DrawingWidget() override;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void drawBezierPoint(const QVector3D &point);

    void drawBezierLine(const QVector3D &start, const QVector3D &end);

    [[nodiscard]] QPointF dropPointToScreen(const QVector3D &point) const;

    [[nodiscard]] QPixmap *getPixMap() const {
        return m_pixMap;
    }

    // ------------------------------
    // Class slots
    // ------------------------------
public slots:
    void updateScene();

    void clearContent();

    void setObserverDistance(double distance);

    void setPixmap(const QPixmap *pixmap) const;

    // ------------------------------
    // Class signals
    // ------------------------------
signals:
    void onElementsUpdate(DrawingWidget *sender);

    // ------------------------------
    // Protected methods
    // ------------------------------
protected:
    void resizeEvent(QResizeEvent *event) override;

    // ------------------------------
    // Protected methods
    // ------------------------------

protected:
    void updateElements();

    void _drawBezierPoint(const QVector3D &point, size_t idx) const;

    void _drawBezierLine(const std::pair<QVector3D, QVector3D> &line) const;

    void _drawTriangleLine(const std::pair<QVector3D, QVector3D> &line) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    QGraphicsScene *m_scene{};
    StateMgr *m_objectMgr{};
    QPixmap *m_pixMap{};
    QGraphicsPixmapItem *m_pixMapItem{};

    float m_width{};
    float m_height{};

    double m_observerDistance{};

    std::vector<QVector3D> m_points{};
    std::vector<std::pair<QVector3D, QVector3D> > m_lines{};
};

#endif //APP_DRAWINGWIDGET_H
