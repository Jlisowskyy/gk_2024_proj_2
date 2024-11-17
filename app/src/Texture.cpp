//
// Created by Jlisowskyy on 11/8/24.
//

#include "../include/Rendering/Texture.h"

Texture::Texture(QObject *parent, const float ksCoef, const float kdCoef, const float mCoef,
                 const QColor &lightColor) : QObject(parent),
                                             m_ksCoef(ksCoef),
                                             m_kdCoef(kdCoef),
                                             m_mCoef(mCoef),
                                             m_lightColor(lightColor) {
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
