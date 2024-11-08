//
// Created by Jlisowskyy on 08/10/24.
//

/* external includes */
#include <QDebug>
#include <string>

/* internal includes */
#include "../include/UiObjects/ToolBar.h"
#include "../include/GraphicObjects/DrawingWidget.h"
#include "../include/UiObjects/DoubleSlider.h"
#include "../include/UiObjects/TextButton.h"

ToolBar::ToolBar(QObject *parent) : QObject(parent) {
}

ToolBar::~ToolBar() = default;

void ToolBar::_addToolbarLiteral(const char *strLiteral) {
    const auto pliteral = new QAction(tr(strLiteral), m_toolBar);
    pliteral->setDisabled(true);
    m_toolBar->addAction(pliteral);
}

QAction *
ToolBar::_addButtonToToolbar(const char *name, const char *imgPath, const char *toolTip) {
    const auto pButton = new QAction(tr(name), m_toolBar);
    pButton->setIcon(QIcon(imgPath));
    pButton->setToolTip(tr(toolTip));
    m_toolBar->addAction(pButton);

    return pButton;
}

void ToolBar::_addSeparator() {
    const auto pLiteral = new QAction(m_toolBar);
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

    m_triangulationSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                             SLIDER_CONSTANTS::TRIANGULATION::MIN,
                                             SLIDER_CONSTANTS::TRIANGULATION::MAX,
                                             SLIDER_CONSTANTS::TRIANGULATION::STEPS,
                                             SLIDER_CONSTANTS::TRIANGULATION::DEFAULT_STEP,
                                             "Triangulation accuracy",
                                             "Change accuracy of triangulation for rendered plain");
    m_toolBar->addWidget(m_triangulationSlider->getContainer());

    m_observerDistanceSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                                SLIDER_CONSTANTS::OBSERVER::MIN,
                                                SLIDER_CONSTANTS::OBSERVER::MAX,
                                                SLIDER_CONSTANTS::OBSERVER::STEPS,
                                                SLIDER_CONSTANTS::OBSERVER::DEFAULT_STEP,
                                                "Observer distance",
                                                "Change distance of observer from the plain");
    m_toolBar->addWidget(m_observerDistanceSlider->getContainer());

    auto pButton = new TextButton(m_toolBar,
                                  "Load Bezier points to the program!",
                                  "Load bezier points",
                                  ":/icons/load_icon.png");
    m_loadBezierPointsButton = pButton->getAction();
    m_toolBar->addWidget(pButton);

    pButton = new TextButton(m_toolBar,
                             "Change whether draw the net over the plain or not",
                             "Draw net",
                             ":/icons/net_icon.png");
    m_drawNetButton = pButton->getAction();
    m_drawNetButton->setCheckable(true);
    m_drawNetButton->setChecked(true);
    m_toolBar->addWidget(pButton);

    pButton = new TextButton(m_toolBar,
                             "Load texture to the program!",
                             "Load texture",
                             ":/icons/load_icon.png");
    m_loadTextureButton = pButton->getAction();
    m_toolBar->addWidget(pButton);

    pButton = new TextButton(m_toolBar,
                             "Draw the texture on the plain!",
                             "Draw the texture",
                             ":/icons/texture_icon.png");
    m_enableTextureButton = pButton->getAction();
    m_enableTextureButton->setCheckable(true);
    m_toolBar->addWidget(pButton);

    pButton = new TextButton(m_toolBar,
                             "Load normal vector to the program!",
                             "Load normal vectors",
                             ":/icons/load_icon.png");
    m_loadNormalVectorsButton = pButton->getAction();
    m_toolBar->addWidget(pButton);

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

    m_alphaSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                     SLIDER_CONSTANTS::ALPHA::MIN,
                                     SLIDER_CONSTANTS::ALPHA::MAX,
                                     SLIDER_CONSTANTS::ALPHA::STEPS,
                                     SLIDER_CONSTANTS::ALPHA::DEFAULT_STEP,
                                     "Alpha angle",
                                     "Z Axis rotation angle");
    m_toolBar->addWidget(m_alphaSlider->getContainer());

    m_betaSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                    SLIDER_CONSTANTS::BETA::MIN,
                                    SLIDER_CONSTANTS::BETA::MAX,
                                    SLIDER_CONSTANTS::BETA::STEPS,
                                    SLIDER_CONSTANTS::BETA::DEFAULT_STEP,
                                    "Beta angle",
                                    "X Axis rotation angle");
    m_toolBar->addWidget(m_betaSlider->getContainer());

    // Lightning section
    _addSeparator();
    _addToolbarLiteral("Lightning options:");

    m_ksSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                  SLIDER_CONSTANTS::KS::MIN,
                                  SLIDER_CONSTANTS::KS::MAX,
                                  SLIDER_CONSTANTS::KS::STEPS,
                                  SLIDER_CONSTANTS::KS::DEFAULT_STEP,
                                  "Ks coefficient",
                                  "Ks coefficient for lighting equation");
    m_toolBar->addWidget(m_ksSlider->getContainer());

    m_kdSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                  SLIDER_CONSTANTS::KD::MIN,
                                  SLIDER_CONSTANTS::KD::MAX,
                                  SLIDER_CONSTANTS::KD::STEPS,
                                  SLIDER_CONSTANTS::KD::DEFAULT_STEP,
                                  "Kd coefficient",
                                  "Kd coefficient for lightning equation");
    m_toolBar->addWidget(m_kdSlider->getContainer());

    m_mSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                 SLIDER_CONSTANTS::M::MIN,
                                 SLIDER_CONSTANTS::M::MAX,
                                 SLIDER_CONSTANTS::M::STEPS,
                                 SLIDER_CONSTANTS::M::DEFAULT_STEP,
                                 "M coefficient",
                                 "M coefficient for lighting equation");
    m_toolBar->addWidget(m_mSlider->getContainer());

    m_lightningPositionSlider = new DoubleSlider(Qt::Horizontal, m_toolBar,
                                                 SLIDER_CONSTANTS::LIGHT_POSITION::MIN,
                                                 SLIDER_CONSTANTS::LIGHT_POSITION::MAX,
                                                 SLIDER_CONSTANTS::LIGHT_POSITION::STEPS,
                                                 SLIDER_CONSTANTS::LIGHT_POSITION::DEFAULT_STEP,
                                                 "Light position",
                                                 "Position of lighting equation");
    m_toolBar->addWidget(m_lightningPositionSlider->getContainer());

    pButton = new TextButton(m_toolBar,
                             "Stop movement of light source!",
                             "Stop light movement",
                             ":/icons/stop_icon.png");
    m_stopLightMovementButton = pButton->getAction();
    m_stopLightMovementButton->setCheckable(true);
    m_stopLightMovementButton->setChecked(true);
    m_toolBar->addWidget(pButton);

    pButton = new TextButton(m_toolBar,
                             "Change color of the plain!",
                             "Change color",
                             ":/icons/color_icon.png");
    m_changePlainColorButton = pButton->getAction();
    m_toolBar->addWidget(pButton);

    pButton = new TextButton(m_toolBar,
                             "Change color of the light source!",
                             "Change light color",
                             ":/icons/color_icon.png");
    m_changeLightColorButton = pButton->getAction();
    m_toolBar->addWidget(pButton);
}
