//
// Created by Jlisowskyy on 11/8/24.
//

#include "../include/Rendering/Texture.h"

Texture::Texture(QObject *parent, const float ksCoef, const float kdCoef, const float mCoef,
                 const QColor &lightColor, const bool useReflector, const float reflector_coef) : QObject(parent),
    m_ksCoef(ksCoef),
    m_kdCoef(kdCoef),
    m_mCoef(mCoef),
    m_lightColor(lightColor),
    m_reflectorCoef(reflector_coef),
    m_drawReflector(useReflector) {
}

QColor Texture::_applyLightToTriangleColor(const QColor &color, const QVector3D &normalVector,
                                           const QVector3D &pos, const QVector3D &lightPos) const {
    static constexpr QVector3D V(0, 0, 1);

    const QVector3D L1 = (lightPos - pos).normalized();
    const QVector3D N = normalVector.normalized();
    const float NdotL1 = QVector3D::dotProduct(N, L1);
    const QVector3D R1 = (2.0f * NdotL1 * N - L1).normalized();

    const float cos00 = std::max(0.0f, NdotL1);
    const float cos10 = std::max(0.0f, QVector3D::dotProduct(V, R1));
    const float cos1m0 = std::pow(cos10, m_mCoef);

    const QVector3D lightPos2 = QVector3D(
        -lightPos.x(), -lightPos.y(), lightPos.z());

    const QVector3D L2 = (lightPos2 - pos).normalized();
    const float NdotL2 = QVector3D::dotProduct(N, L2);
    const QVector3D R2 = (2.0f * NdotL2 * N - L2).normalized();

    const float cos01 = std::max(0.0f, NdotL2);
    const float cos11 = std::max(0.0f, QVector3D::dotProduct(V, R2));
    const float cos1m1 = std::pow(cos11, m_mCoef);

    QVector3D lightColors = QVector3D(
                                static_cast<float>(m_lightColor.red()),
                                static_cast<float>(m_lightColor.green()),
                                static_cast<float>(m_lightColor.blue())) / 255.0f;

    QVector3D objColors = QVector3D(
                              static_cast<float>(color.red()),
                              static_cast<float>(color.green()),
                              static_cast<float>(color.blue())) / 255.0f;

    const float a1 = QVector3D::dotProduct(L1.normalized(), lightPos.normalized());
    const float a2 = QVector3D::dotProduct(L2.normalized(), lightPos2.normalized());

    const float a1m = std::abs(std::pow(a1, m_reflectorCoef));
    const float a2m = std::abs(std::pow(a2, m_reflectorCoef));

    QVector3D resultColors{};
    for (int i = 0; i < 3; ++i) {
        const float left = m_kdCoef * lightColors[i] * objColors[i] * cos00;
        const float right = m_ksCoef * lightColors[i] * objColors[i] * cos1m0;

        const float left1 = m_kdCoef * lightColors[i] * objColors[i] * cos01;
        const float right1 = m_ksCoef * lightColors[i] * objColors[i] * cos1m1;

        const float light = m_drawReflector
                                ? (left + right) * a1m + (left1 + right1) * a2m
                                : left + right + left1 + right1;

        resultColors[i] = std::clamp(light, 0.0f, 1.0f);
    }

    resultColors *= 255.0f;

    return {
        static_cast<int>(resultColors.x()),
        static_cast<int>(resultColors.y()),
        static_cast<int>(resultColors.z())
    };
}

Texture::_drawData Texture::_preprocess(const Triangle &triangle) {
    _drawData result{};

    result.v0 = triangle[1].rotatedPosition - triangle[0].rotatedPosition;
    result.v1 = triangle[2].rotatedPosition - triangle[0].rotatedPosition;

    result.d00 = QVector3D::dotProduct(result.v0, result.v0);
    result.d01 = QVector3D::dotProduct(result.v0, result.v1);
    result.d11 = QVector3D::dotProduct(result.v1, result.v1);

    return result;
}

void Texture::_drawLineOwn(const QVector3D &from, const QVector3D &to, BitMap &bitMap, int16_t *zBuffer) {
    int x1 = int(from.x() + bitMap.width() / 2.0);
    int y1 = int(from.y() + bitMap.height() / 2.0);
    int x2 = int(to.x() + bitMap.width() / 2.0);
    int y2 = int(to.y() + bitMap.height() / 2.0);

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    float z1 = from.z();
    float z2 = to.z();

    while (true) {
        if (x1 >= 0 && y1 >= 0 && x1 < bitMap.width() && y1 < bitMap.height()) {
            float t = (dx > dy) ?
                     float(x1 - (from.x() + bitMap.width() / 2.0)) / (to.x() - from.x()) :
                     float(y1 - (from.y() + bitMap.height() / 2.0)) / (to.y() - from.y());
            float z = z1 + t * (z2 - z1) + 2.0;

            auto zRounded = static_cast<int16_t>(std::floor(z));
            if (zRounded > zBuffer[y1 * bitMap.width() + x1]) {
                zBuffer[y1 * bitMap.width() + x1] = zRounded;
                bitMap.setColorAt(x1, y1, QColor(0, 0, 0));
            }
        }

        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

QVector3D Texture::_findNormal(const QVector3D &pos, const Triangle &triangle) const {
    QVector3D v0 = triangle[1].position - triangle[0].position;
    QVector3D v1 = triangle[2].position - triangle[0].position;
    QVector3D v2 = pos - triangle[0].position;

    float d00 = QVector3D::dotProduct(v0, v0);
    float d01 = QVector3D::dotProduct(v0, v1);
    float d11 = QVector3D::dotProduct(v1, v1);
    float d20 = QVector3D::dotProduct(v2, v0);
    float d21 = QVector3D::dotProduct(v2, v1);

    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    QVector3D interpolatedNormal =
        triangle[0].normal * u +
        triangle[1].normal * v +
        triangle[2].normal * w;

    return interpolatedNormal.normalized();
}
