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

/* internal includes */
#include "../Constants.h"
#include "../PrimitiveData/ActiveEdge.h"
#include "../PrimitiveData/Triangle.h"

/* Forward declaration */
class ObjectMgr;

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

    void drawTriangleLines(const QVector3D &start, const QVector3D &end);

    [[nodiscard]] QPointF dropPointToScreen(const QVector3D &point) const;

    void setTriangles(std::vector<Triangle> *triangles);

    template<typename ColorGetT, typename ColorSetT, size_t N>
    void colorPolygon(ColorGetT colorGet, ColorSetT colorSet, const PolygonArr<N> &polygon);

    void setColor(const QColor &color);

    void setFillType(FillType fillType);

    // void setTexture();

    // void setNormals();

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

    void _drawBezierPoint(const QVector3D &point, size_t idx) const;

    void _drawBezierLine(const std::pair<QVector3D, QVector3D> &line) const;

    void _drawTriangleLine(const std::pair<QVector3D, QVector3D> &line) const;

    void _fillTriangle(const Triangle &triangle);

    // ------------------------------
    // Class fields
    // ------------------------------

    QGraphicsScene *m_scene{};
    ObjectMgr *m_objectMgr{};
    double m_observerDistance{};
    QPixmap *m_pixMap{};
    QColor m_color{};
    FillType m_fillType{};

    std::vector<QVector3D> m_points{};
    std::vector<std::pair<QVector3D, QVector3D> > m_lines{};
    std::vector<std::pair<QVector3D, QVector3D> > m_triangleLines{};
    std::vector<Triangle> *m_triangles{};
};

template<typename ColorGetT, typename ColorSetT, size_t N>
void DrawingWidget::colorPolygon(ColorGetT colorGet, ColorSetT colorSet, const PolygonArr<N> &polygon) {
    QPainter painter(m_pixMap);

    /* sort the vertices by y cord */
    std::array<size_t, N> sorted{};
    for (size_t i = 0; i < N; i++) {
        sorted[i] = i;
    }

    for (int i = 1; i < N; i++) {
        size_t key = sorted[i];
        int j = i - 1;

        while (j >= 0 && polygon[sorted[j]].position.y() > polygon[key].position.y()) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    std::list<ActiveEdge> aet{};
    Vertex vBuffer{};
    int y = static_cast<int>(std::round(polygon[sorted[0]].position.y()));
    size_t nextVertex = 0;

    while (nextVertex < N || !aet.empty()) {
        /* Detect et edges on the current y */
        while (nextVertex < N && static_cast<int>(std::round(polygon[sorted[nextVertex]].position.y())) == y) {
            const size_t curr = sorted[nextVertex];
            const size_t prev = (curr + N - 1) % N;
            const size_t next = (curr + 1) % N;

            if (std::round(polygon[prev].position.y()) > y) {
                aet.emplace_back(
                    polygon[curr].position,
                    polygon[prev].position
                );
            }

            if (std::round(polygon[next].position.y()) > y) {
                aet.emplace_back(
                    polygon[curr].position,
                    polygon[next].position
                );
            }

            nextVertex++;
        }

        /* sort the aet edges by the x coordinate */
        aet.sort([](const ActiveEdge &a, const ActiveEdge &b) { return a.x < b.x; });

        /* Draw the pixels */
        auto it = aet.begin();
        while (it != aet.end() && std::next(it) != aet.end()) {
            int x1 = static_cast<int>(std::round(it->x));
            int x2 = static_cast<int>(std::round(std::next(it)->x));

            for (int x = x1; x <= x2; x++) {
                vBuffer.position.setX(static_cast<float>(x));
                vBuffer.position.setY(static_cast<float>(y));

                /* Process color values */
                QColor color = colorGet(vBuffer.position);
                color = colorSet(vBuffer, color);

                painter.setPen(QPen(color));
                painter.drawPoint(x, y);
            }

            std::advance(it, 2);
        }

        ++y;

        /* Update x coordinates for each edge */
        for (auto &edge: aet) {
            edge.x += edge.dx;
        }

        aet.remove_if([y](const ActiveEdge &edge) {
            return y >= edge.yMax;
        });
    }

    painter.end();
}


#endif //APP_DRAWINGWIDGET_H
