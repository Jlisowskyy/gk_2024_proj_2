//
// Created by Jlisowskyy on 07/11/24.
//

/* internal includes */
#include "../include/Rendering/BitMap.h"

/* external includes */
#include <memory>
#include <QPainter>
#include <QPen>
#include <QColor>

BitMap::BitMap(const int32_t width, const int32_t height): m_redMap(static_cast<_baseTypeT *>(malloc(
                                                               sizeof(_baseTypeT) * width * height))),
                                                           m_greenMap(static_cast<_baseTypeT *>(malloc(
                                                               sizeof(_baseTypeT) * width * height))),
                                                           m_blueMap(static_cast<_baseTypeT *>(malloc(
                                                               sizeof(_baseTypeT) * width * height))),
                                                           m_width(width),
                                                           m_height(height) {
}

BitMap::~BitMap() {
    free(m_redMap);
    free(m_greenMap);
    free(m_blueMap);
}

void BitMap::setWhiteAll() {
    for (size_t i = 0; i < m_width * m_height; i++) {
        m_redMap[i] = m_greenMap[i] = m_blueMap[i] = 255;
    }
}

void BitMap::dropToPixMap(QPixmap &pixMap) const {
    const QImage image = createQImage();
    pixMap = QPixmap::fromImage(image);
}

QImage BitMap::createQImage() const {
    QImage image(m_width, m_height, QImage::Format_RGB32);

    #pragma omp parallel for schedule(static)
    for (int32_t y = 0; y < m_height; y++) {
        uchar *line = image.scanLine(y);
        for (int32_t x = 0; x < m_width; x++) {
            const size_t pixelIndex = _atCord(x, y, m_width);
            const size_t lineIndex = x * 4;

            line[lineIndex + 0] = static_cast<uchar>(m_blueMap[pixelIndex]);
            line[lineIndex + 1] = static_cast<uchar>(m_greenMap[pixelIndex]);
            line[lineIndex + 2] = static_cast<uchar>(m_redMap[pixelIndex]);
            line[lineIndex + 3] = 255;
        }
    }

    return image;
}
