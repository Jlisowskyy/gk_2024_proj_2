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

Texture::_drawData Texture::_preprocess(const Triangle &triangle) {
    _drawData result{};

    result.v0 = triangle[1].rotatedPosition - triangle[0].rotatedPosition;
    result.v1 = triangle[2].rotatedPosition - triangle[0].rotatedPosition;

    result.d00 = QVector3D::dotProduct(result.v0, result.v0);
    result.d01 = QVector3D::dotProduct(result.v0, result.v1);
    result.d11 = QVector3D::dotProduct(result.v1, result.v1);

    result.denom = result.d00 * result.d11 - result.d01 * result.d01;

    return result;
}

Texture::vAVXPointst
Texture::_applyLightToTriangleColorAVX(Texture::vAVXPointst &colors, Texture::vAVXPointst &normalVectors,
                                    Texture::vAVXPointst &pos, const QVector3D &lightPos) const {
    static constexpr QVector3D V(0, 0, 1);

    _normalizeAVX(normalVectors);

    alignas(32) vAVXPointst toLightVectors{};

    for (int i = 0; i < 3; ++i) {
        *cast(toLightVectors[i]) = _mm256_sub_ps(*cast(pos[i]), _mm256_set1_ps(lightPos[i]));
    }

    alignas(32) vAVX NdotL{};
    for (int i = 0; i < 3; ++i) {
        *cast(NdotL) = _mm256_fmadd_ps(*cast(normalVectors[i]), *cast(toLightVectors[i]), *cast(NdotL));
    }

    alignas(32) vAVXPointst R{};
    alignas(32) vAVX NdotL2{};
    *cast(NdotL2) = _mm256_mul_ps(*cast(NdotL), _mm256_set1_ps(2.0f));

    for (int i = 0; i < 3; ++i) {
        *cast(R[i]) = _mm256_fmsub_ps(*cast(NdotL2), *cast(normalVectors[i]), *cast(toLightVectors[i]));
    }

    _normalizeAVX(R);

    alignas(32) vAVX VdotR{};

    for (int i = 0; i < 3; ++i) {
        *cast(VdotR) = _mm256_fmadd_ps(_mm256_set1_ps(V[i]), *cast(R[i]), *cast(VdotR));
    }

    alignas(32) vAVX cos0{};
    *cast(cos0) = _mm256_max_ps(*cast(NdotL), _mm256_set1_ps(0.0f));

    alignas(32) vAVX cos1{};
    *cast(cos1) = _mm256_max_ps(*cast(VdotR), _mm256_set1_ps(0.0f));

    alignas(32) vAVX cos1m{};
    *cast(cos1m) = _mm256_set1_ps(1.0f);
    for (int i = 0; i < static_cast<int>(m_mCoef); ++i) {
        *cast(cos1m) = _mm256_mul_ps(*cast(cos1m), *cast(cos1));
    }

    QVector3D lightColors = QVector3D(
            static_cast<float>(m_lightColor.red()),
            static_cast<float>(m_lightColor.green()),
            static_cast<float>(m_lightColor.blue())) / 255.0f;

    alignas(32) vAVXPointst lightColorsAVX{};

    for (int i = 0; i < 3; ++i) {
        alignas(32) vAVX left{};
        *cast(left) = _mm256_mul_ps(_mm256_set1_ps(m_kdCoef), _mm256_set1_ps(lightColors[i]));
        *cast(left) = _mm256_mul_ps(*cast(left), *cast(colors[i]));
        *cast(left) = _mm256_mul_ps(*cast(left), *cast(cos0));

        alignas(32) vAVX right{};
        *cast(right) = _mm256_mul_ps(_mm256_set1_ps(m_ksCoef), _mm256_set1_ps(lightColors[i]));
        *cast(right) = _mm256_mul_ps(*cast(right), *cast(colors[i]));
        *cast(right) = _mm256_mul_ps(*cast(right), *cast(cos1m));

        *cast(lightColorsAVX[i]) = _mm256_add_ps(*cast(left), *cast(right));
        *cast(lightColorsAVX[i]) = _mm256_max_ps(*cast(lightColorsAVX[i]), _mm256_set1_ps(0.0f));
        *cast(lightColorsAVX[i]) = _mm256_min_ps(*cast(lightColorsAVX[i]), _mm256_set1_ps(1.0f));

        *cast(lightColorsAVX[i]) = _mm256_mul_ps(*cast(lightColorsAVX[i]), _mm256_set1_ps(255.0f));
    }

    return lightColorsAVX;
}

void Texture::_normalizeAVX(Texture::vAVXPointst &points) const {
    alignas(32) vAVX lengths{};

    for (int i = 0; i < 3; ++i) {
        *cast(lengths) = _mm256_fmadd_ps(*cast(points[i]), *cast(points[i]), *cast(lengths));
    }

    *cast(lengths) = _mm256_sqrt_ps(*cast(lengths));

    for (int i = 0; i < 3; ++i) {
        *cast(points[i]) = _mm256_div_ps(*cast(points[i]), *cast(lengths));
    }
}

