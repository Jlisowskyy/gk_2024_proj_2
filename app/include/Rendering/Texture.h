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
#include <QMatrix3x3>

class Texture : public QObject {
    Q_OBJECT

public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Texture(QObject *parent, float ksCoef, float kdCoef, float mCoef, const QColor &lightColor, bool useReflector,
            float reflectorCoef);

    ~Texture() override = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template<bool useNormals, typename ColorGetterT>
    void fillPixmap(QPixmap &pixmap, const Mesh &mesh, ColorGetterT colorGetter, const QVector3D &lightPos) const;

    template<bool useNormals, typename ColorGetterT, size_t N>
    void colorPolygon(BitMap &bitMap, int16_t *zBuffer, ColorGetterT colorGet, const PolygonArr<N> &polygon,
                      const QVector3D &lightPos) const;

    template<bool useNormals, size_t N>
    void colorFigure(BitMap &bitMap, int16_t *zBuffer, QColor color, const PolygonArr<N> &polygon,
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

    void setNormalMap(QImage *image) {
        if (image == m_normalMap) {
            return;
        }

        delete m_normalMap;
        m_normalMap = image;
    }

    void setDrawNet(const bool drawNet) {
        m_drawNet = drawNet;
    }

    void setUseReflector(const bool useReflector) {
        m_drawReflector = useReflector;
    }

    void setReflectorCoef(const float reflectorCoef) {
        m_reflectorCoef = reflectorCoef;
    }

    // ------------------------------
    // Class protected methods
    // ------------------------------
protected:
    struct _drawData {
        QVector3D v0;
        QVector3D v1;

        float d00;
        float d01;
        float d11;
    };

    template<bool useNormals>
    [[nodiscard]] std::tuple<float, float, QVector3D>
    _interpolateFromTrianglePoint(const QVector3D &pos, const Triangle &triangle, const _drawData &drawData) const;

    [[nodiscard]] QColor _applyLightToTriangleColor(const QColor &color, const QVector3D &normalVector,
                                                    const QVector3D &pos, const QVector3D &lightPos) const;

    template<bool useNormals, typename ColorGetterT>
    [[nodiscard]] QColor _processColor(ColorGetterT colorGetter, const QVector3D &pos, const Triangle &triangle,
                                       const QVector3D &lightPos, const _drawData &drawData) const;

    [[nodiscard]] static _drawData _preprocess(const Triangle &triangle);

    static void _drawLineOwn(const QVector3D &from, const QVector3D &to, BitMap &bitMap, int16_t *zBuffer);

    QVector3D _findNormal(const QVector3D &pos, const Triangle &triangle) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    float m_ksCoef{};
    float m_kdCoef{};
    float m_mCoef{};
    QColor m_lightColor{};

    bool m_drawNet{};

    QImage *m_normalMap{};

    float m_reflectorCoef{};
    bool m_drawReflector{};
};

template<bool useNormals, typename ColorGetterT>
void Texture::fillPixmap(QPixmap &pixmap, const Mesh &mesh, ColorGetterT colorGetter, const QVector3D &lightPos) const {
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
        colorPolygon<useNormals>(bitMap, zBuffer, colorGetter, triangle, lightPos);
    }

    if (m_drawNet) {
        for (const auto &triangle: mesh.getMeshArr()) {
            for (size_t i = 0; i < 3; i++) {
                const auto &v1 = triangle[i].rotatedPosition;
                const auto &v2 = triangle[(i + 1) % 3].rotatedPosition;

                _drawLineOwn(v1, v2, bitMap, zBuffer);
            }
        }
    }

    size_t idx = 0;
    for (const auto &triangle: mesh.getFigure()) {
        const QColor color = mesh.getFigureColor(idx++);
        colorFigure<false>(bitMap, zBuffer, color, triangle, lightPos);

        for (size_t i = 0; i < 3; i++) {
            const auto &v1 = triangle[i].rotatedPosition;
            const auto &v2 = triangle[(i + 1) % 3].rotatedPosition;

            _drawLineOwn(v1, v2, bitMap, zBuffer);
        }
    }

    bitMap.dropToPixMap(pixmap);

    const auto t1 = std::chrono::steady_clock::now();
    const auto t = t1 - t0;

    qDebug() << "Time spent on drawing texture: " << t.count() << " ns";

    const auto tm = std::chrono::duration_cast<std::chrono::milliseconds>(t);

    QPainter painter(&pixmap);

    painter.setPen(QPen(Qt::black, 2));

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

template<bool useNormals, typename ColorGetterT, size_t N>
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

    /* works only for triangles */
    _drawData drawData = _preprocess(polygon);

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

                        const QColor color = _processColor<
                            useNormals>(colorGet, drawPoint, polygon, lightPos, drawData);
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

                    const QColor color = _processColor<useNormals>(colorGet, drawPoint, polygon, lightPos, drawData);
                    bitMap.setColorAt(screenX, screenY, color);
                }
            }
        }
    }
}

template<bool useNormals, size_t N>
void Texture::colorFigure(BitMap &bitMap, int16_t *zBuffer, QColor color, const PolygonArr<N> &polygon,
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

    /* works only for triangles */
    _drawData drawData = _preprocess(polygon);

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

                        color = _applyLightToTriangleColor(color, _findNormal(drawPoint, polygon),
                                                           drawPoint, lightPos);
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

                    color = _applyLightToTriangleColor(color, _findNormal(drawPoint, polygon),
                                                       drawPoint, lightPos);
                    bitMap.setColorAt(screenX, screenY, color);
                }
            }
        }
    }
}

template<bool useNormals, typename ColorGetterT>
QColor Texture::_processColor(ColorGetterT colorGetter, const QVector3D &pos, const Triangle &triangle,
                              const QVector3D &lightPos, const _drawData &drawData) const {
    const auto [u, v, interpolatedNormalVector] = _interpolateFromTrianglePoint<useNormals>(pos, triangle, drawData);
    const QColor color = colorGetter(u, v);
    return _applyLightToTriangleColor(color, interpolatedNormalVector, pos, lightPos);
}

template<bool useNormals>
std::tuple<float, float, QVector3D>
Texture::_interpolateFromTrianglePoint(const QVector3D &pos, const Triangle &triangle,
                                       const _drawData &drawData) const {
    const QVector3D v2 = pos - triangle[0].rotatedPosition;

    const float d20 = QVector3D::dotProduct(v2, drawData.v0);
    const float d21 = QVector3D::dotProduct(v2, drawData.v1);

    const float denom = drawData.d00 * drawData.d11 - drawData.d01 * drawData.d01;
    const float v = (drawData.d11 * d20 - drawData.d01 * d21) / denom;
    const float w = (drawData.d00 * d21 - drawData.d01 * d20) / denom;
    const float u = 1.0f - v - w;

    const float interpolatedU =
            std::clamp(u * triangle[0].u + v * triangle[1].u + w * triangle[2].u, 0.0f, 1.0f);
    const float interpolatedV =
            std::clamp(u * triangle[0].v + v * triangle[1].v + w * triangle[2].v, 0.0f, 1.0f);

    QVector3D interpolatedNormalVector =
            (u * triangle[0].rotatedNormal + v * triangle[1].rotatedNormal + w * triangle[2].rotatedNormal);

    if constexpr (useNormals) {
        const QColor color = m_normalMap->pixelColor(
            static_cast<int>(interpolatedV * static_cast<float>(m_normalMap->width() - 1)),
            static_cast<int>((1.0f - interpolatedU) * static_cast<float>(m_normalMap->height() - 1))
        );

        QVector3D normalFromTexture(
            (color.red() - 127.0f) / 127.0f,
            (color.green() - 127.0f) / 127.0f,
            (color.blue() - 127.0f) / 127.0f
        );
        normalFromTexture.normalize();

        QVector3D interpolatedPU = (u * triangle[0].rotatedPuVector +
                                    v * triangle[1].rotatedPuVector +
                                    w * triangle[2].rotatedPuVector);
        interpolatedPU.normalize();

        QVector3D interpolatedPV = (u * triangle[0].rotatedPvVector +
                                    v * triangle[1].rotatedPvVector +
                                    w * triangle[2].rotatedPvVector);
        interpolatedPV.normalize();
        interpolatedNormalVector.normalize();

        interpolatedNormalVector = QVector3D(
            interpolatedPU.x() * normalFromTexture.x() +
            interpolatedPV.x() * normalFromTexture.y() +
            interpolatedNormalVector.x() * normalFromTexture.z(),

            interpolatedPU.y() * normalFromTexture.x() +
            interpolatedPV.y() * normalFromTexture.y() +
            interpolatedNormalVector.y() * normalFromTexture.z(),

            interpolatedPU.z() * normalFromTexture.x() +
            interpolatedPV.z() * normalFromTexture.y() +
            interpolatedNormalVector.z() * normalFromTexture.z()
        );

        interpolatedNormalVector.normalize();
    }

    return {interpolatedU, interpolatedV, interpolatedNormalVector};
}


#endif //TEXTURE_H
