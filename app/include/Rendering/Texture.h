//
// Created by Jlisowskyy on 11/8/24.
//

#ifndef TEXTURE_H
#define TEXTURE_H

/* internal includes */
#include "../Intf.h"
#include "../Rendering/Mesh.h"

/* external includes */
#include <QObject>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QPixmap>

class Texture : public QObject {
    Q_OBJECT

public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Texture(QObject *parent, float ksCoef, float kdCoef, float mCoef, const QColor &lightColor);

    ~Texture() override = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template<typename ColorGetterT>
    void fillPixmap(QPixmap &pixmap, const Mesh &mesh, ColorGetterT colorGetter, const QVector3D &lightPos) const;

    template<typename ColorGetterT, size_t N>
    void colorPolygon(QPixmap &pixmap, ColorGetterT colorGet, const PolygonArr<N> &polygon,
                      const QVector3D &lightPos) const;

    // ------------------------------
    // Public slots
    // ------------------------------

public slots:
    void setLightColor(const QColor &lightColor) {
        m_lightColor = lightColor;
    }

    void setKsCoef(const float ksCoef) {
        m_ksCoef = ksCoef;
    }

    void setKdCoef(const float kdCoef) {
        m_kdCoef = kdCoef;
    }

    void setMCoef(const float mCoef) {
        m_mCoef = mCoef;
    }

    // ------------------------------
    // Class protected methods
    // ------------------------------
protected:
    [[nodiscard]] static std::tuple<float, float, QVector3D>
    _interpolateFromTrianglePoint(const QVector3D &pos, const Triangle &triangle);

    [[nodiscard]] QColor _applyLightToTriangleColor(const QColor &color, const QVector3D &normalVector,
                                                    const QVector3D &pos, const QVector3D &lightPos) const;

    template<typename ColorGetterT>
    [[nodiscard]] QColor _processColor(ColorGetterT colorGetter, const QVector3D &pos, const Triangle &triangle,
                                       const QVector3D &lightPos) const;

    // ------------------------------
    // Class fields
    // ------------------------------


    float m_ksCoef{};
    float m_kdCoef{};
    float m_mCoef{};
    QColor m_lightColor{};
};

template<typename colorGet>
void Texture::fillPixmap(QPixmap &pixmap, const Mesh &mesh, colorGet colorGetter, const QVector3D &lightPos) const {
    pixmap.fill(Qt::white);

    // #pragma omp parallel for schedule(static)
    for (const auto &triangle: mesh.getMeshArr()) {
        colorPolygon(pixmap, colorGetter, triangle, lightPos);
    }
}

template<typename ColorGetterT, size_t N>
void Texture::colorPolygon(QPixmap &pixmap, ColorGetterT colorGet, const PolygonArr<N> &polygon,
                           const QVector3D &lightPos) const {
    QPainter painter(&pixmap);

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
                const QColor color = _processColor(colorGet, vBuffer.position, polygon, lightPos);
                QVector3D screenPos(vBuffer.position.x() + static_cast<float>(pixmap.width()) / 2.0f,
                                    vBuffer.position.y() + static_cast<float>(pixmap.height()) / 2.0f,
                                    0.0f);

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
        const auto &v1 = polygon[i].rotatedPosition;
        const auto &v2 = polygon[(i + 1) % N].rotatedPosition;

        if (std::abs(v1.y() - v2.y()) <= 1.0f) {
            const int y = static_cast<int>(std::floor(v1.y()));
            const int x1 = static_cast<int>(std::floor(std::min(v1.x(), v2.x())));
            const int x2 = static_cast<int>(std::floor(std::max(v1.x(), v2.x())));

            for (int x = x1; x <= x2; x++) {
                vBuffer.position.setX(static_cast<float>(x));
                vBuffer.position.setY(static_cast<float>(y));
                vBuffer.position.setZ(100);

                const QColor color = _processColor(colorGet, vBuffer.position, polygon, lightPos);
                QVector3D screenPos(vBuffer.position.x() + static_cast<float>(pixmap.width()) / 2.0f,
                                    vBuffer.position.y() + static_cast<float>(pixmap.height()) / 2.0f,
                                    0.0f);

                painter.setPen(QPen(color));
                painter.drawPoint(static_cast<int>(screenPos.x()), static_cast<int>(screenPos.y()));
            }
        }
    }

    painter.end();
}

template<typename ColorGetterT>
QColor Texture::_processColor(ColorGetterT colorGetter, const QVector3D &pos, const Triangle &triangle,
                              const QVector3D &lightPos) const {
    const auto [u, v, interpolatedNormalVector] = _interpolateFromTrianglePoint(pos, triangle);
    const QColor color = colorGetter(u, v);
    return _applyLightToTriangleColor(color, interpolatedNormalVector, pos, lightPos);
}


#endif //TEXTURE_H
