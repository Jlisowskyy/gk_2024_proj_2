//
// Created by Jlisowskyy on 07/11/24.
//

#ifndef APP_BITMAP_H
#define APP_BITMAP_H

/* internal includes */

/* external includes */
#include <cinttypes>
#include <QColor>
#include <QPixmap>

class BitMap {
    using _baseTypeT = uint8_t;

    // ------------------------------
    // Class creation
    // ------------------------------
public:
    explicit BitMap(int32_t width, int32_t height);

    ~BitMap();

    // ------------------------------
    // Class interaction
    // ------------------------------

    void setWhiteAll();

    [[nodiscard]] QColor colorAt(const int32_t x, const int32_t y) const {
        return {
            m_redMap[_atCord(x, y, m_width)],
            m_greenMap[_atCord(x, y, m_height)],
            m_blueMap[_atCord(x, y, m_width)]
        };
    }

    void setRedAt(const int32_t x, const int32_t y, const _baseTypeT red) {
        m_redMap[_atCord(x, y, m_width)] = red;
    }

    void setGreenAt(const int32_t x, const int32_t y, const _baseTypeT green) {
        m_greenMap[_atCord(x, y, m_height)] = green;
    }

    void setBlueAt(const int32_t x, const int32_t y, const _baseTypeT blue) {
        m_blueMap[_atCord(x, y, m_height)] = blue;
    }

    void setColorAt(const int32_t x, const int32_t y, const _baseTypeT red, const _baseTypeT green,
                    const _baseTypeT blue) {
        m_redMap[_atCord(x, y, m_width)] = red;
        m_greenMap[_atCord(x, y, m_height)] = green;
        m_blueMap[_atCord(x, y, m_width)] = blue;
    }

    void dropToPixMap(QPixmap& pixMap) const;

    // ------------------------------
    // Protected class methods
    // ------------------------------


protected:
    static constexpr int32_t _atCord(const int32_t x, const int32_t y, const int32_t width) {
        return y * width + x;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    _baseTypeT *m_redMap{};
    _baseTypeT *m_greenMap{};
    _baseTypeT *m_blueMap{};

    int32_t m_width{};
    int32_t m_height{};
};


#endif //APP_BITMAP_H
