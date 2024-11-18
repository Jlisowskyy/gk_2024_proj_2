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

    ~SceneMgr() override = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] FillType getFillType() const;

    void redrawScene(DrawingWidget &drawingWidget, const Texture &texture, const Mesh &mesh);

    void bondWithComponents(DrawingWidget *drawingWidget, Texture *texture, Mesh *mesh);

    void unbound();

    // ------------------------------
    // Class public slots
    // ------------------------------

public slots:
    void setColor(const QColor &color);

    void setIsAnimationPlayed(bool isAnimationPlaying);

    void setDrawNet(bool drawNet);

    void setUseTexture(bool useTexture);

    void setTextureImg(QImage *image);

    void setLightZ(int z);

    void setLightColor(const QColor &color);

    void setNormalMap(QImage *image);

    void setUseNormals(bool useNormals);

    // ------------------------------
    // Class protected methods
    // ------------------------------
protected slots:
    void _onTimer();

    void _onElementsUpdate(const DrawingWidget *sender);

protected:
    static void _drawNet(DrawingWidget &drawingWidget, const Mesh &mesh);

    template<bool drawNormals>
    void _drawTexture(const DrawingWidget &drawingWidget, const Texture &texture, const Mesh &mesh);

    void _processLightPosition();

    [[nodiscard]] QPointF _getLightPosition2D() const;

    [[nodiscard]] QVector3D _getLightPos() const;

    void _addLightItem(const DrawingWidget *drawingWidget);

    void _drawTextureWithNormals(const DrawingWidget &drawingWidget, const Texture &texture, const Mesh &mesh);

    // ------------------------------
    // Class fields
    // ------------------------------

    /* Settings fields */
    bool m_useTexture{};
    bool m_isAnimationPlaying{};
    bool m_drawNet{};
    bool m_useNormals{};

    /* connected objects */
    bool m_isBound{};
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

    QGraphicsEllipseItem *m_lightEllipse{};
    QGraphicsEllipseItem *m_lightEllipse1{};

    QImage *m_normalMap{};
};

#endif //SCENEMGR_H
