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
    QPainter painter(&pixMap);

    for (int32_t x = 0; x < m_width; x++) {
        for (int32_t y = 0; y < m_height; y++) {
            painter.setPen(QPen(colorAt(x, y)));
            painter.drawPoint(x, y);
        }
    }
}
