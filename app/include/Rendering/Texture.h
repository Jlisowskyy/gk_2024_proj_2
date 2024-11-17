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

#include <immintrin.h>

class Texture : public QObject {
    Q_OBJECT

    using vAVX = std::array<float, 8>;
    using vAVXPointst = std::array<vAVX, 3>;
public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Texture(QObject *parent, float ksCoef, float kdCoef, float mCoef, const QColor &lightColor);

    ~Texture() override = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template<bool useNormals, typename ColorGetterT>
    void fillPixmap(QPixmap &pixmap, const Mesh &mesh, ColorGetterT colorGetter, const QVector3D &lightPos) const;

    template<bool useNormals, typename ColorGetterT, size_t N>
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

    void setNormalMap(QImage *image) {
        if (image == m_normalMap) {
            return;
        }

        delete m_normalMap;
        m_normalMap = image;
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

        float denom;
    };

    template<bool useNormals>
    [[nodiscard]] std::tuple<float, float, QVector3D>
    _interpolateFromTrianglePoint(const QVector3D &pos, const Triangle &triangle, const _drawData& drawData) const;

    template<bool useNormals>
    [[nodiscard]] std::tuple<vAVX, vAVX, vAVXPointst>
    _interpolateFromTrianglePointAVX(vAVXPointst &points, const Triangle &triangle, const _drawData &drawData) const;

    [[nodiscard]] QColor _applyLightToTriangleColor(const QColor &color, const QVector3D &normalVector,
                                                    const QVector3D &pos, const QVector3D &lightPos) const;

    [[nodiscard]] vAVXPointst _applyLightToTriangleColorAVX(vAVXPointst &colors, vAVXPointst &normalVectors,
                                                         vAVXPointst &pos, const QVector3D &lightPos) const;

    template<bool useNormals, typename ColorGetterT>
    [[nodiscard]] QColor _processColor(ColorGetterT colorGetter, const QVector3D &pos, const Triangle &triangle,
                                       const QVector3D &lightPos, const _drawData& drawData) const;

    template<bool useNormals, typename ColorGetterT>
    [[nodiscard]] vAVXPointst _processColorAVX(ColorGetterT colorGetter, vAVXPointst &points, const Triangle &triangle,
                                            const QVector3D &lightPos, const _drawData &drawData, int numValues) const;


    [[nodiscard]] static _drawData _preprocess(const Triangle& triangle);

    [[nodiscard]] static __m256 *cast(vAVX &points) {
        return reinterpret_cast<__m256 *>(points.data());
    }

    void _normalizeAVX(vAVXPointst &points) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    float m_ksCoef{};
    float m_kdCoef{};
    float m_mCoef{};
    QColor m_lightColor{};

    QImage *m_normalMap{};
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

//    #pragma omp parallel for schedule(static)
    for (const auto &triangle: mesh.getMeshArr()) {
        colorPolygon<useNormals>(bitMap, zBuffer, colorGetter, triangle, lightPos);
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

            for (int x = x1; x <= x2;) {
                const int dist = x2 - x;
                const int offset = std::clamp(dist, 1, 8);

                alignas(32) vAVXPointst points{};

                for (int i = 0; i < offset; ++i) {
                    points[0][i] = static_cast<float>(x + i);
                }

                for (int i = 0; i < offset; ++i) {
                    points[1][i] = static_cast<float>(scanLineY);
                }

                for (int i = 0; i < offset; ++i) {
                    points[2][i] = zLeft + static_cast<float>(x - x1 + i) * zStep;
                }

                alignas(32) const auto colors = _processColorAVX<useNormals>(colorGet, points, polygon, lightPos, drawData, offset);

                alignas(32) vAVX screenX{};
                *cast(screenX) = _mm256_add_ps(*cast(points[0]),
                                               _mm256_set1_ps(static_cast<float>(bitMap.width()) / 2.0f));

                alignas(32) vAVX screenY{};
                *cast(screenY) = _mm256_add_ps(*cast(points[1]), _mm256_set1_ps(
                        static_cast<float>(scanLineY) + static_cast<float>(bitMap.height()) / 2.0f));

                for (int i = 0; i < offset; ++i) {
                    QVector3D pos = QVector3D(points[0][i], points[1][i], points[2][i]);

                    if (screenX[i] >= 0 && screenX[i] < bitMap.width() && screenY[i] >= 0 &&
                        screenY[i] < bitMap.height()) {
                        if (const auto zRounded = static_cast<int16_t>(std::floor(pos.z()));
                                zRounded >
                                zBuffer[static_cast<int>(screenY[i]) * bitMap.width() + static_cast<int>(screenX[i])]) {

                            zBuffer[static_cast<int>(screenY[i]) * bitMap.width() +
                                    static_cast<int>(screenX[i])] = zRounded;


                        }
                    }

                }

                x += offset;
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

//    for (size_t i = 0; i < N; i++) {
//        const auto &v1 = polygon[i].rotatedPosition;
//        const auto &v2 = polygon[(i + 1) % N].rotatedPosition;
//
//        if (std::abs(v1.y() - v2.y()) <= 1.0f) {
//            const int y = static_cast<int>(std::floor(v1.y()));
//            const int x1 = static_cast<int>(std::floor(std::min(v1.x(), v2.x())));
//            const int x2 = static_cast<int>(std::floor(std::max(v1.x(), v2.x())));
//
//            const float zStart = v1.z();
//            const float zEnd = v2.z();
//            const float zStep = x2 - x1 != 0 ? (zEnd - zStart) / (x2 - x1) : 0.0f;
//
//            for (int x = x1; x <= x2; ++x) {
//                float z = zStart + (x - x1) * zStep;
//
//                const int screenX = x + bitMap.width() / 2;
//                const int screenY = y + bitMap.height() / 2;
//
//                if (screenX >= 0 && screenX < bitMap.width() && screenY >= 0 && screenY < bitMap.height()) {
//                    const QVector3D drawPoint{
//                        static_cast<float>(x),
//                        static_cast<float>(scanLineY),
//                        z
//                    };
//
//                    const QColor color = _processColor<useNormals>(colorGet, drawPoint, polygon, lightPos, drawData);
//                    bitMap.setColorAt(screenX, screenY, color);
//                }
//            }
//        }
//    }

    return;
}

template<bool useNormals, typename ColorGetterT>
QColor Texture::_processColor(ColorGetterT colorGetter, const QVector3D &pos, const Triangle &triangle,
                              const QVector3D &lightPos, const _drawData& drawData) const {
    const auto [u, v, interpolatedNormalVector] = _interpolateFromTrianglePoint<useNormals>(pos, triangle, drawData);
    const QColor color = colorGetter(u, v);
    return _applyLightToTriangleColor(color, interpolatedNormalVector, pos, lightPos);
}

template<bool useNormals, typename ColorGetterT>
Texture::vAVXPointst
Texture::_processColorAVX(ColorGetterT colorGetter, Texture::vAVXPointst &points, const Triangle &triangle,
                       const QVector3D &lightPos, const Texture::_drawData &drawData, int numValues) const {
    alignas(32) auto [u, v, interpolatedNormalVector] = _interpolateFromTrianglePointAVX<useNormals>(points, triangle, drawData);

    alignas(32) vAVXPointst colors{};

    for (int i = 0; i < numValues; ++i) {
        const QColor color = colorGetter(u[i], v[i]);

        colors[0][i] = static_cast<float>(color.red());
        colors[1][i] = static_cast<float>(color.green());
        colors[2][i] = static_cast<float>(color.blue());
    }

    return _applyLightToTriangleColorAVX(colors, interpolatedNormalVector, points, lightPos);
}

template<bool useNormals>
std::tuple<float, float, QVector3D>
Texture::_interpolateFromTrianglePoint(const QVector3D &pos, const Triangle &triangle, const _drawData& drawData ) const {
    const QVector3D v2 = pos - triangle[0].rotatedPosition;

    const float d20 = QVector3D::dotProduct(v2, drawData.v0);
    const float d21 = QVector3D::dotProduct(v2, drawData.v1);

    const float v = (drawData.d11 * d20 - drawData.d01 * d21) / drawData.denom;
    const float w = (drawData.d00 * d21 - drawData.d01 * d20) / drawData.denom;
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
                (color.blue() - 128.0f) / 127.0f
        );

        const QVector3D interpolatedPU = (u * triangle[0].rotatedPuVector +
                                          v * triangle[1].rotatedPuVector +
                                          w * triangle[2].rotatedPuVector);

        const QVector3D interpolatedPV = (u * triangle[0].rotatedPvVector +
                                          v * triangle[1].rotatedPvVector +
                                          w * triangle[2].rotatedPvVector);

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

template<bool useNormals>
std::tuple<Texture::vAVX, Texture::vAVX, Texture::vAVXPointst>
Texture::_interpolateFromTrianglePointAVX(Texture::vAVXPointst &points, const Triangle &triangle,
                                       const Texture::_drawData &drawData) const {
    alignas(32) vAVXPointst v2{};

    for (int i = 0; i < 3; ++i) {
        *cast(v2[i]) = _mm256_sub_ps(*cast(points[i]), _mm256_set1_ps(triangle[0].rotatedPosition[i]));
    }

    alignas(32) vAVX d20{};

    for (int i = 0; i < 3; ++i) {
        *cast(d20) = _mm256_fmadd_ps(*cast(v2[i]), _mm256_set1_ps(drawData.v0[i]), *cast(d20));
    }

    alignas(32) vAVX d21{};

    for (int i = 0; i < 3; ++i) {
        *cast(d21) = _mm256_fmadd_ps(*cast(v2[i]), _mm256_set1_ps(drawData.v1[i]), *cast(d21));
    }

    alignas(32) vAVX vV{};

    *cast(vV) = _mm256_sub_ps(_mm256_mul_ps(_mm256_set1_ps(drawData.d11), *cast(d20)),
                              _mm256_mul_ps(_mm256_set1_ps(drawData.d01), *cast(d21)));
    *cast(vV) = _mm256_div_ps(*cast(vV), _mm256_set1_ps(drawData.denom));

    alignas(32) vAVX vW{};

    *cast(vW) = _mm256_sub_ps(_mm256_mul_ps(_mm256_set1_ps(drawData.d00), *cast(d21)),
                              _mm256_mul_ps(_mm256_set1_ps(drawData.d01), *cast(d20)));
    *cast(vW) = _mm256_div_ps(*cast(vW), _mm256_set1_ps(drawData.denom));

    alignas(32) vAVX vU{};
    *cast(vU) = _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_add_ps(*cast(vV), *cast(vW)));

    alignas(32) vAVX interpolatedU{};
    *cast(interpolatedU) = _mm256_fmadd_ps(*cast(vU), _mm256_set1_ps(triangle[0].u), *cast(interpolatedU));
    *cast(interpolatedU) = _mm256_fmadd_ps(*cast(vV), _mm256_set1_ps(triangle[1].u), *cast(interpolatedU));
    *cast(interpolatedU) = _mm256_fmadd_ps(*cast(vW), _mm256_set1_ps(triangle[2].u), *cast(interpolatedU));

    alignas(32) vAVX interpolatedV{};
    *cast(interpolatedV) = _mm256_fmadd_ps(*cast(vU), _mm256_set1_ps(triangle[0].v), *cast(interpolatedV));
    *cast(interpolatedV) = _mm256_fmadd_ps(*cast(vV), _mm256_set1_ps(triangle[1].v), *cast(interpolatedV));
    *cast(interpolatedV) = _mm256_fmadd_ps(*cast(vW), _mm256_set1_ps(triangle[2].v), *cast(interpolatedV));

    for (int i = 0; i < 8; ++i) {
        interpolatedU[i] = std::clamp(interpolatedU[i], 0.0f, 1.0f);
        interpolatedV[i] = std::clamp(interpolatedV[i], 0.0f, 1.0f);
    }

    alignas(32) vAVXPointst interpolatedNormalVector{};

    for (int i = 0; i < 3; ++i) {
        *cast(interpolatedNormalVector[i]) = _mm256_fmadd_ps(*cast(vU), _mm256_set1_ps(triangle[0].rotatedNormal[i]),
                                                             *cast(interpolatedNormalVector[i]));
        *cast(interpolatedNormalVector[i]) = _mm256_fmadd_ps(*cast(vV), _mm256_set1_ps(triangle[1].rotatedNormal[i]),
                                                             *cast(interpolatedNormalVector[i]));
        *cast(interpolatedNormalVector[i]) = _mm256_fmadd_ps(*cast(vW), _mm256_set1_ps(triangle[2].rotatedNormal[i]),
                                                             *cast(interpolatedNormalVector[i]));
    }

    return {interpolatedU, interpolatedV, interpolatedNormalVector};
}


#endif //TEXTURE_H
