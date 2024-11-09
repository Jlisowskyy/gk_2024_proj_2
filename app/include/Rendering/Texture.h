//
// Created by Jlisowskyy on 11/8/24.
//

#ifndef TEXTURE_H
#define TEXTURE_H

/* internal includes */
#include "../Intf.h"
#include "../Rendering/Mesh.h"
#include "../Rendering/BitMap.h"

/* external includes */
#include <QObject>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <chrono>
#include <QDebug>

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
    void colorPolygon(BitMap &bitMap, int16_t *zBuffer, ColorGetterT colorGet, const PolygonArr<N> &polygon,
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
    const auto t0 = std::chrono::steady_clock::now();
    const size_t zBufferSize = pixmap.width() * pixmap.height();

    const auto zBuffer = static_cast<int16_t *>(malloc(sizeof(int16_t) * zBufferSize));
    for (size_t z = 0; z < zBufferSize; ++z) {
        zBuffer[z] = INT16_MIN;
    }

    BitMap bitMap(pixmap.width(), pixmap.height());
    bitMap.setWhiteAll();

    #pragma omp parallel for schedule(static)
    for (const auto &triangle: mesh.getMeshArr()) {
        colorPolygon(bitMap, zBuffer, colorGetter, triangle, lightPos);
    }

    bitMap.dropToPixMap(pixmap);

    const auto t1 = std::chrono::steady_clock::now();
    const auto t = t1 - t0;

    qDebug() << "Time spent on drawing texture: " << t.count() << " ns";

    const auto tm = std::chrono::duration_cast<std::chrono::milliseconds>(t);

    QPainter painter(&pixmap);
    QFont font{};
    font.setFamily("Courier");
    font.setPointSize(UI_CONSTANTS::DEFAULT_FPS_SIZE);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);

    painter.drawText(0, 20, "Fps: " + QString::number(1000.0 / static_cast<double>(tm.count())));

    free(zBuffer);
}

template<typename ColorGetterT, size_t N>
void Texture::colorPolygon(BitMap &bitMap, int16_t *zBuffer, ColorGetterT colorGet, const PolygonArr<N> &polygon,
                           const QVector3D &lightPos) const {
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
    int scanLineY = static_cast<int>(std::floor(polygon[sorted[0]].rotatedPosition.y()));
    size_t nextVertex = 0;

    while (nextVertex < N || !aet.empty()) {
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

        aet.sort([](const ActiveEdge &a, const ActiveEdge &b) {
            return a.x < b.x || (a.x == b.x && a.dx < b.dx);
        });

        auto it = aet.begin();
        while (it != aet.end() && std::next(it) != aet.end()) {
            int x1 = static_cast<int>(std::floor(it->x));
            int x2 = static_cast<int>(std::ceil(std::next(it)->x));

            float zLeft = it->z;
            float zRight = std::next(it)->z;
            float zStep = (x2 - x1) != 0 ? (zRight - zLeft) / static_cast<float>(x2 - x1) : 0.0f;

            for (int x = x1; x <= x2; x++) {
                float z = zLeft + static_cast<float>(x - x1) * zStep;

                const int screenX = x + bitMap.width() / 2;
                const int screenY = scanLineY + bitMap.height() / 2;

                if (screenX >= 0 && screenX < bitMap.width() && screenY >= 0 && screenY < bitMap.height()) {
                    if (const auto zRounded = static_cast<int16_t>(std::floor(z));
                        zRounded > zBuffer[screenY * bitMap.width() + screenX]) {
                        zBuffer[screenY * bitMap.width() + screenX] = zRounded;

                        const QVector3D drawPoint{
                            static_cast<float>(x),
                            static_cast<float>(scanLineY),
                            z
                        };

                        const QColor color = _processColor(colorGet, drawPoint, polygon, lightPos);
                        bitMap.setColorAt(screenX, screenY, color);
                    }
                }
            }

            std::advance(it, 2);
        }

        ++scanLineY;

        for (auto &edge: aet) {
            edge.update();
        }

        aet.remove_if([scanLineY](const ActiveEdge &edge) {
            return scanLineY >= edge.yMax;
        });
    }

    for (size_t i = 0; i < N; i++) {
        const auto &v1 = polygon[i].rotatedPosition;
        const auto &v2 = polygon[(i + 1) % N].rotatedPosition;

        if (std::abs(v1.y() - v2.y()) <= 1.0f) {
            const int y = static_cast<int>(std::floor(v1.y()));
            const int x1 = static_cast<int>(std::floor(std::min(v1.x(), v2.x())));
            const int x2 = static_cast<int>(std::floor(std::max(v1.x(), v2.x())));

            const float zStart = v1.z();
            const float zEnd = v2.z();
            const float zStep = x2 - x1 != 0 ? (zEnd - zStart) / (x2 - x1) : 0.0f;

            for (int x = x1; x <= x2; ++x) {
                float z = zStart + (x - x1) * zStep;

                const int screenX = x + bitMap.width() / 2;
                const int screenY = y + bitMap.height() / 2;

                if (screenX >= 0 && screenX < bitMap.width() && screenY >= 0 && screenY < bitMap.height()) {
                    const QVector3D drawPoint{
                        static_cast<float>(x),
                        static_cast<float>(scanLineY),
                        z
                    };

                    const QColor color = _processColor(colorGet, drawPoint, polygon, lightPos);
                    bitMap.setColorAt(screenX, screenY, color);
                }
            }
        }
    }
}

template<typename ColorGetterT>
QColor Texture::_processColor(ColorGetterT colorGetter, const QVector3D &pos, const Triangle &triangle,
                              const QVector3D &lightPos) const {
    const auto [u, v, interpolatedNormalVector] = _interpolateFromTrianglePoint(pos, triangle);
    const QColor color = colorGetter(u, v);
    return _applyLightToTriangleColor(color, interpolatedNormalVector, pos, lightPos);
}


#endif //TEXTURE_H
