//
// Created by Jlisowskyy on 11/8/24.
//

#include "../include/Rendering/Texture.h"

Texture::Texture(QObject *parent, const float ksCoef, const float kdCoef, const float mCoef,
                 const QColor &lightColor) : QObject(parent),
                                             m_kdCoef(kdCoef),
                                             m_ksCoef(ksCoef),
                                             m_mCoef(mCoef),
                                             m_lightColor(lightColor) {
}

std::tuple<float, float, QVector3D> Texture::_interpolateFromTrianglePoint(const QVector3D &pos,
                                                                           const Triangle &triangle) {
    const QVector3D v0 = triangle[1].rotatedPosition - triangle[0].rotatedPosition;
    const QVector3D v1 = triangle[2].rotatedPosition - triangle[0].rotatedPosition;
    const QVector3D v2 = pos - triangle[0].rotatedPosition;

    const float d00 = QVector3D::dotProduct(v0, v0);
    const float d01 = QVector3D::dotProduct(v0, v1);
    const float d11 = QVector3D::dotProduct(v1, v1);
    const float d20 = QVector3D::dotProduct(v2, v0);
    const float d21 = QVector3D::dotProduct(v2, v1);

    const float denom = d00 * d11 - d01 * d01;
    const float v = (d11 * d20 - d01 * d21) / denom;
    const float w = (d00 * d21 - d01 * d20) / denom;
    const float u = 1.0f - v - w;

    const float interpolatedU =
            std::clamp(u * triangle[0].u + v * triangle[1].u + w * triangle[2].u, 0.0f, 1.0f);
    const float interpolatedV =
            std::clamp(u * triangle[0].v + v * triangle[1].v + w * triangle[2].v, 0.0f, 1.0f);

    QVector3D interpolatedNormalVector =
            (u * triangle[0].rotatedNormal + v * triangle[1].rotatedNormal + w * triangle[2].rotatedNormal);

    return {interpolatedU, interpolatedV, interpolatedNormalVector};
}

QColor Texture::_applyLightToTriangleColor(const QColor &color, const QVector3D &normalVector,
                                           const QVector3D &pos, const QVector3D &lightPos) const {
    static constexpr QVector3D V(0, 0, 1);

    const QVector3D L = (lightPos - pos).normalized();
    const QVector3D N = normalVector.normalized();
    const float NdotL = QVector3D::dotProduct(N, L);
    const QVector3D R = (2.0f * NdotL * N - L).normalized();

    const float cos0 = std::max(0.0f, NdotL);
    const float cos1 = std::max(0.0f, QVector3D::dotProduct(V, R));
    const float cos1m = std::pow(cos1, m_mCoef);

    QVector3D lightColors = QVector3D(
                                static_cast<float>(m_lightColor.red()),
                                static_cast<float>(m_lightColor.green()),
                                static_cast<float>(m_lightColor.blue())) / 255.0f;

    QVector3D objColors = QVector3D(
                              static_cast<float>(color.red()),
                              static_cast<float>(color.green()),
                              static_cast<float>(color.blue())) / 255.0f;

    QVector3D resultColors{};
    for (int i = 0; i < 3; ++i) {
        const float left = m_kdCoef * lightColors[i] * objColors[i] * cos0;
        const float right = m_ksCoef * lightColors[i] * objColors[i] * cos1m;
        resultColors[i] = std::clamp(left + right, 0.0f, 1.0f);
    }

    resultColors *= 255.0f;

    return {
        static_cast<int>(resultColors.x()),
        static_cast<int>(resultColors.y()),
        static_cast<int>(resultColors.z())
    };
}
