//
// Created by Jlisowskyy on 11/8/24.
//

#ifndef SCENEMGR_H
#define SCENEMGR_H

/* internal includes */
#include "../Intf.h"

/* external includes */
#include <QObject>
#include <QImage>
#include <QColor>
#include <QTimer>
#include <QGraphicsEllipseItem>

#include "../GraphicObjects/DrawingWidget.h"


/* Forward Declarations */
class Mesh;
class Texture;
class DrawingWidget;

class SceneMgr final : public QObject {
    Q_OBJECT

    // ------------------------------
    // Class creation
    // ------------------------------
public:
    explicit SceneMgr(QObject *parent,
                      const QColor &color,
                      bool drawNet,
                      bool useTexture,
                      bool playAnimation,
                      const QColor &lightColor,
                      int lightZ,
                      QImage *image = nullptr);

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] FillType getFillType() const;

    void redrawScene(DrawingWidget &drawingWidget, const Texture &texture, const Mesh &mesh);

    void setupLightAnim(DrawingWidget *drawingWidget, Texture *texture, Mesh *mesh);

    // ------------------------------
    // Class public slots
    // ------------------------------

public slots:
    void setColor(const QColor &color) {
        m_color = color;
    }

    void setIsAnimationPlayed(const bool isAnimationPlayed) {
        m_isAnimationPlayed = isAnimationPlayed;
    }

    void setDrawNet(const bool drawNet) {
        m_drawNet = drawNet;
    }

    void setUseTexture(const bool useTexture) {
        m_useTexture = useTexture;

        m_fillType = getFillType();
    }

    void setTextureImg(QImage *image) {
        delete m_textureImg;
        m_textureImg = image;

        m_fillType = getFillType();
    }

    void setLightZ(const int z) {
        m_lightZ = z;
    }

    void setLightColor(const QColor &color) {
        m_lightColor = color;
    }

    // ------------------------------
    // Class protected methods
    // ------------------------------
protected slots:
    void _onTimer();

    void _addLightItem(const DrawingWidget *sender);

protected:
    static void _drawNet(DrawingWidget &drawingWidget, const Mesh &mesh);

    void _drawTexture(const DrawingWidget &drawingWidget, const Texture &texture, const Mesh &mesh);

    void _processLightPosition();

    [[nodiscard]] QPointF _getLightPosition2D() const;

    [[nodiscard]] QVector3D _getLightPos() const;

    // ------------------------------
    // Class fields
    // ------------------------------

    /* Settings fields */
    bool m_useTexture{};
    bool m_isAnimationPlayed{};
    bool m_drawNet{};

    /* connected objects */
    DrawingWidget *m_drawingWidget{};
    Texture *m_texture{};
    Mesh *m_mesh{};

    /* Object state */
    FillType m_fillType;

    /* color source for texture */
    QImage *m_textureImg{};
    QColor m_color{};

    /* light components */
    QTimer *m_timer{};
    int m_lightZ{};
    float m_lightPos{};
    QColor m_lightColor{};
    QGraphicsEllipseItem *m_lightEllipse{};
};

#endif //SCENEMGR_H
