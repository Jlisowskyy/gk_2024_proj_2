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

    template<typename ColorGetT, size_t N>
    void colorPolygon(ColorGetT colorGet, const PolygonArr<N> &polygon);


    // void setNormals();

    // ------------------------------
    // Class slots
    // ------------------------------
public slots:
    void updateScene();

    void clearContent();

    void setObserverDistance(double distance);

    void setColor(const QColor &color);

    void setFillType(FillType fillType);

    void setTexture(QImage *texture);

    void setTriangles(std::vector<Triangle> *triangles);

    void setKsCoef(float value);

    void setKdCoef(float value);

    void setMCoef(float value);

    void setLightZ(int value);

    void setStopLight(bool value);

    void setLightColor(const QColor &color);

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

protected slots:

    void _onTimer();

protected:
    void updateElements();

    void _drawBezierPoint(const QVector3D &point, size_t idx) const;

    void _drawBezierLine(const std::pair<QVector3D, QVector3D> &line) const;

    void _drawTriangleLine(const std::pair<QVector3D, QVector3D> &line) const;

    void _fillTriangle(const Triangle &triangleToFill);

    [[nodiscard]] QColor _getTextureColor(const QVector3D &pos, const Triangle &triangle) const;

    [[nodiscard]] QColor _getPlainColor(const QVector3D &pos, const PolygonArr<3> &polygon) const;

    void _setupLight();

    void _processLightPosition();

    void _drawTexture();

    void _addLightDrawing();

    [[nodiscard]] QPointF _getLightPosition2D() const;

    [[nodiscard]] QVector3D _getLightPosition3D() const;

    [[nodiscard]] static std::tuple<float, float, QVector3D>
    _interpolateFromTrianglePoint(const QVector3D &pos, const Triangle &triangle);

    [[nodiscard]] QColor _applyLightToTriangleColor(const QColor &color, const QVector3D &normalVector, const QVector3D &pos) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    QGraphicsScene *m_scene{};
    ObjectMgr *m_objectMgr{};
    QPixmap *m_pixMap{};
    QTimer *m_timer{};
    QGraphicsEllipseItem *m_lightEllipse{};
    QGraphicsPixmapItem *m_pixMapItem{};

    float m_width{};
    float m_height{};
    QColor m_color{};
    FillType m_fillType{};
    QImage *m_texture{};

    float m_ksCoef{};
    float m_kdCoef{};
    float m_mCoef{};
    int m_lightZ{};
    float m_lightPos{};
    bool m_stopLight{};
    QColor m_lightColor{};

    double m_observerDistance{};
    std::vector<QVector3D> m_points{};
    std::vector<std::pair<QVector3D, QVector3D> > m_lines{};
    std::vector<std::pair<QVector3D, QVector3D> > m_triangleLines{};
    std::vector<Triangle> *m_triangles{};
};

template<typename ColorGetT, size_t N>
void DrawingWidget::colorPolygon(ColorGetT colorGet, const PolygonArr<N> &polygon) {
    QPainter painter(m_pixMap);

    /* sort the vertices by y coordinate */
    std::array<size_t, N> sorted{};
    for (size_t i = 0; i < N; i++) {
        sorted[i] = i;
    }

    for (int i = 1; i < static_cast<int>(N); i++) {
        size_t key = sorted[i];
        int j = i - 1;

        while (j >= 0 && polygon[sorted[j]].rotatedPosition.y() > polygon[key].rotatedPosition.y()) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    std::list<ActiveEdge> aet{};
    Vertex vBuffer{};
    int scanLineY = static_cast<int>(std::floor(polygon[sorted[0]].rotatedPosition.y()));
    size_t nextVertex = 0;

    while (nextVertex < N || !aet.empty()) {
        /* Detect et edges on the current y */
        while (nextVertex < N &&
               static_cast<int>(std::floor(polygon[sorted[nextVertex]].rotatedPosition.y())) == scanLineY) {
            const size_t curr = sorted[nextVertex];
            const size_t prev = (curr + N - 1) % N;
            const size_t next = (curr + 1) % N;

            const float currY = polygon[curr].rotatedPosition.y();
            const float prevY = polygon[prev].rotatedPosition.y();
            const float nextY = polygon[next].rotatedPosition.y();

            if (prevY > currY) {
                aet.emplace_back(
                        polygon[prev].rotatedPosition,
                        polygon[curr].rotatedPosition
                );
            }

            if (nextY > currY) {
                aet.emplace_back(
                        polygon[next].rotatedPosition,
                        polygon[curr].rotatedPosition
                );
            }

            nextVertex++;
        }

        /* sort the aet edges by the x coordinate */
        aet.sort([](const ActiveEdge &a, const ActiveEdge &b) {
            return a.x < b.x || (a.x == b.x && a.dx < b.dx);
        });

        /* Draw the pixels */
        auto it = aet.begin();
        while (it != aet.end() && std::next(it) != aet.end()) {
            int x1 = static_cast<int>(std::floor(it->x));
            int x2 = static_cast<int>(std::ceil(std::next(it)->x));

            for (int x = x1; x <= x2; x++) {
                vBuffer.position.setX(static_cast<float>(x));
                vBuffer.position.setY(static_cast<float>(scanLineY));
                vBuffer.position.setZ(100);

                /* Process color values */
                QColor color = colorGet(vBuffer.position, polygon);
                QVector3D screenPos(vBuffer.position.x() + m_width / 2,
                                    vBuffer.position.y() + m_height / 2,
                                    0);

                painter.setPen(QPen(color));
                painter.drawPoint(static_cast<int>(screenPos.x()), static_cast<int>(screenPos.y()));
            }

            std::advance(it, 2);
        }

        ++scanLineY;

        /* Update x coordinates for each edge */
        for (auto &edge: aet) {
            edge.x += edge.dx;
        }

        aet.remove_if([scanLineY](const ActiveEdge &edge) {
            return scanLineY >= edge.yMax;
        });
    }

     /* WTF: HORIZONTAL EDGES? */
    for (size_t i = 0; i < N; i++) {
        const auto& v1 = polygon[i].rotatedPosition;
        const auto& v2 = polygon[(i + 1) % N].rotatedPosition;

        if (std::abs(v1.y() - v2.y()) <= 1.0f) {
            const int y = static_cast<int>(std::floor(v1.y()));
            const int x1 = static_cast<int>(std::floor(std::min(v1.x(), v2.x())));
            const int x2 = static_cast<int>(std::floor(std::max(v1.x(), v2.x())));

            for (int x = x1; x <= x2; x++) {
                vBuffer.position.setX(static_cast<float>(x));
                vBuffer.position.setY(static_cast<float>(y));

                QColor color = colorGet(vBuffer.position, polygon);
                QVector3D screenPos(vBuffer.position.x() + m_width / 2,
                                    vBuffer.position.y() + m_height / 2,
                                    0);

                painter.setPen(QPen(color));
                painter.drawPoint(static_cast<int>(screenPos.x()), static_cast<int>(screenPos.y()));
            }
        }
    }

    painter.end();
}


#endif //APP_DRAWINGWIDGET_H
