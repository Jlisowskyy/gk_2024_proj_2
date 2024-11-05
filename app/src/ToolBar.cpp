//
// Created by Jlisowskyy on 08/10/24.
//

/* external includes */
#include <QDebug>
#include <string>

/* internal includes */
#include "../include/ManagingObjects/ToolBar.h"
#include "../include/GraphicObjects/DrawingWidget.h"
#include "../include/UiObjects/DoubleSlider.h"
#include "../include/UiObjects/TextButton.h"

ToolBar::ToolBar(QObject *parent) : QObject(parent) {
}

ToolBar::~ToolBar() = default;

void ToolBar::_addToolbarLiteral(const char *strLiteral) const {
    auto pliteral = new QAction(tr(strLiteral), m_toolBar);
    pliteral->setDisabled(true);
    m_toolBar->addAction(pliteral);
}

QAction *
ToolBar::_addButtonToToolbar(const char *name, const char *imgPath, const char *toolTip) const {
    auto pButton = new QAction(tr(name), m_toolBar);
    pButton->setIcon(QIcon(imgPath));
    pButton->setToolTip(tr(toolTip));
    m_toolBar->addAction(pButton);

    return pButton;
}

void ToolBar::_addSeparator() {
    auto pLiteral = new QAction(m_toolBar);
    pLiteral->setSeparator(true);
    m_toolBar->addAction(pLiteral);
}

void ToolBar::setupToolBar(QToolBar *toolBar, DrawingWidget *drawingWidget) {
    Q_ASSERT(toolBar != nullptr);
    Q_ASSERT(m_toolBar == nullptr);
    m_toolBar = toolBar;

    Q_ASSERT(m_drawingWidget == nullptr);
    Q_ASSERT(drawingWidget != nullptr);
    m_drawingWidget = drawingWidget;

    _addToolbarLiteral("Drawing options:");
    m_toolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);

    // Triangulation slider
    m_triangulationSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                             TRIANGULATION_SLIDER_MIN, TRIANGULATION_SLIDER_MAX,
                                             TRIANGULATION_SLIDER_STEPS, TRIANGULATION_SLIDER_DEFAULT,
                                             "Triangulation accuracy",
                                             "Change accuracy of triangulation for rendered plain");
    m_toolBar->addWidget(m_triangulationSlider->getContainer());

    // Observer distance slider
    m_observerDistanceSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                                OBSERVER_DISTANCE_MIN, OBSERVER_DISTANCE_MAX,
                                                OBSERVER_DISTANCE_STEPS, DEFAULT_OBSERVER_DISTANCE,
                                                "Observer distance",
                                                "Change distance of observer from the plain");
    m_toolBar->addWidget(m_observerDistanceSlider->getContainer());

    // Load Bezier points button
    auto pButton = new TextButton(m_toolBar,
                                  "Load Bezier points to the program!",
                                  "Load bezier points",
                                  ":/icons/load_icon.png");
    m_loadBezierPointsButton = pButton->getAction();
    m_toolBar->addWidget(pButton);

    // Draw net button
    pButton = new TextButton(m_toolBar,
                             "Change whether draw the net over the plain or not",
                             "Draw net",
                             ":/icons/net_icon.png");
    m_drawNetButton = pButton->getAction();
    m_drawNetButton->setCheckable(true);
    m_toolBar->addWidget(pButton);

    // Load texture button
    pButton = new TextButton(m_toolBar,
                             "Load texture to the program!",
                             "Load texture",
                             ":/icons/load_icon.png");
    m_loadTextureButton = pButton->getAction();
    m_toolBar->addWidget(pButton);

    // Enable texture button
    pButton = new TextButton(m_toolBar,
                             "Draw the texture on the plain!",
                             "Draw the texture",
                             ":/icons/texture_icon.png");
    m_enableTextureButton = pButton->getAction();
    m_enableTextureButton->setCheckable(true);
    m_toolBar->addWidget(pButton);

    // Load normal vectors button
    pButton = new TextButton(m_toolBar,
                             "Load normal vector to the program!",
                             "Load normal vectors",
                             ":/icons/load_icon.png");
    m_loadNormalVectorsButton = pButton->getAction();
    m_toolBar->addWidget(pButton);

    // Enable normal vectors button
    pButton = new TextButton(m_toolBar,
                             "Enable usage of normal vectors in the program",
                             "Enable normal vectors",
                             ":/icons/vector_icon.png");
    m_enableNormalVectorsButton = pButton->getAction();
    m_enableTextureButton->setCheckable(true);
    m_toolBar->addWidget(pButton);

    // Rotation section
    _addSeparator();
    _addToolbarLiteral("Rotations:");

    // Alpha angle slider
    m_alphaSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                     ALPHA_ANGLE_MIN, ALPHA_ANGLE_MAX,
                                     ALPHA_ANGLE_STEPS, ALPHA_ANGLE_DEFAULT,
                                     "Alpha angle",
                                     "Z Axis rotation angle");
    m_toolBar->addWidget(m_alphaSlider->getContainer());

    // Beta angle slider
    m_betaSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                    BETA_ANGLE_MIN, BETA_ANGLE_MAX,
                                    BETA_ANGLE_STEPS, BETA_ANGLE_DEFAULT,
                                    "Beta angle",
                                    "X Axis rotation angle");
    m_toolBar->addWidget(m_betaSlider->getContainer());

    // Lightning section
    _addSeparator();
    _addToolbarLiteral("Lightning options:");

    // Ks coefficient slider
    m_ksSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                  LIGHTING_COEFFICIENT_MIN, LIGHTING_COEFFICIENT_MAX,
                                  LIGHTING_COEFFICIENT_STEPS, LIGHTING_COEFFICIENT_DEFAULT,
                                  "Ks coefficient",
                                  "Ks coefficient for lighting equation");
    m_toolBar->addWidget(m_ksSlider->getContainer());

    // Kd coefficient slider
    m_kdSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                  LIGHTING_COEFFICIENT_MIN, LIGHTING_COEFFICIENT_MAX,
                                  LIGHTING_COEFFICIENT_STEPS, LIGHTING_COEFFICIENT_DEFAULT,
                                  "Kd coefficient",
                                  "Kd coefficient for lightning equation");
    m_toolBar->addWidget(m_kdSlider->getContainer());

    // M coefficient slider
    m_mSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                 M_COEFFICIENT_MIN, M_COEFFICIENT_MAX,
                                 M_COEFFICIENT_STEPS, M_COEFFICIENT_DEFAULT,
                                 "M coefficient",
                                 "M coefficient for lighting equation");
    m_toolBar->addWidget(m_mSlider->getContainer());

    // Light position slider
    m_lightningPositionSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                                 LIGHTING_COEFFICIENT_MIN, LIGHTING_COEFFICIENT_MAX,
                                                 LIGHTING_COEFFICIENT_STEPS, LIGHTING_COEFFICIENT_DEFAULT,
                                                 "Light position",
                                                 "Position of lighting equation");
    m_toolBar->addWidget(m_lightningPositionSlider->getContainer());

    // Stop light movement button
    pButton = new TextButton(m_toolBar,
                             "Stop movement of light source!",
                             "Stop light movement",
                             ":/icons/stop_icon.png");
    m_stopLightMovementButton = pButton->getAction();
    m_stopLightMovementButton->setCheckable(true);
    m_toolBar->addWidget(pButton);

    // Change plain color button
    pButton = new TextButton(m_toolBar,
                             "Change color of the plain!",
                             "Change color",
                             ":/icons/color_icon.png");
    m_changePlainColorButton = pButton->getAction();
    m_toolBar->addWidget(pButton);
}
