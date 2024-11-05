//
// Created by Jlisowskyy on 11/5/24.
//

/* internal includes */
#include "../include/ManagingObjects/ObjectMgr.h"
#include "../include/ManagingObjects/ToolBar.h"
#include "../include/UiObjects/DoubleSlider.h"

/* external includes */
#include <vector>
#include <QColorDialog>
#include <QDebug>


ObjectMgr::ObjectMgr(QObject *parent, QWidget *widgetParent): QObject(parent), m_parentWidget(widgetParent) {
    Q_ASSERT(parent && widgetParent);
}

ObjectMgr::~ObjectMgr() = default;

void ObjectMgr::connectToToolBar(ToolBar *toolBar) {
    /* State changes */
    std::vector<std::pair<DoubleSlider *, void (ObjectMgr::*)(double)> > vSliderProc{
        {toolBar->m_triangulationSlider, &ObjectMgr::onTriangulationChanged},
        {toolBar->m_alphaSlider, &ObjectMgr::onAlphaChanged},
        {toolBar->m_betaSlider, &ObjectMgr::onBetaChanged},
        {toolBar->m_ksSlider, &ObjectMgr::onKSChanged},
        {toolBar->m_kdSlider, &ObjectMgr::onKDChanged},
        {toolBar->m_mSlider, &ObjectMgr::onMChanged},
    };

    for (const auto &[slider, proc]: vSliderProc) {
        connect(slider, &DoubleSlider::doubleValueChanged, this, proc);
    }

    /* simple actions */
    std::vector<std::pair<QAction *, void (ObjectMgr::*)()> > vActionProc{
        {toolBar->m_loadTextureButton, &ObjectMgr::onLoadTexturesTriggered},
        {toolBar->m_loadBezierPointsButton, &ObjectMgr::onLoadBezierPointsTriggered},
        {toolBar->m_loadNormalVectorsButton, &ObjectMgr::onLoadNormalVectorsTriggered},
        {toolBar->m_changePlainColorButton, &ObjectMgr::onColorChangedTriggered},
    };

    for (const auto &[action, proc]: vActionProc) {
        connect(action, &QAction::triggered, this, proc);
    }

    /* Check able actions */
    std::vector<std::pair<QAction *, void (ObjectMgr::*)(bool)> > vActionBoolProc{
        {toolBar->m_drawNetButton, &ObjectMgr::onDrawNetChanged},
        {toolBar->m_enableTextureButton, &ObjectMgr::onEnableTextureChanged},
        {toolBar->m_enableNormalVectorsButton, &ObjectMgr::onEnableNormalVectorsChanged},
        {toolBar->m_stopLightMovementButton, &ObjectMgr::onStopLightingMovementChanged},
    };

    for (const auto &[action, proc]: vActionBoolProc) {
        connect(action, &QAction::triggered, this, proc);
    }
}

void ObjectMgr::loadDefaultSettings() {
    m_color = DEFAULT_PLAIN_COLOR;
}

void ObjectMgr::redraw() {
}

void ObjectMgr::onTriangulationChanged(double value) {
}

void ObjectMgr::onAlphaChanged(double value) {
}

void ObjectMgr::onBetaChanged(double value) {
}

void ObjectMgr::onKSChanged(double value) {
}

void ObjectMgr::onKDChanged(double value) {
}

void ObjectMgr::onMChanged(double value) {
}

void ObjectMgr::onLightningPositionChanged(double value) {
}

void ObjectMgr::onDrawNetChanged(bool isChecked) {
}

void ObjectMgr::onEnableTextureChanged(bool isChecked) {
}

void ObjectMgr::onEnableNormalVectorsChanged(bool isChecked) {
}

void ObjectMgr::onStopLightingMovementChanged(bool isChecked) {
}

void ObjectMgr::onLoadBezierPointsTriggered() {
}

void ObjectMgr::onLoadTexturesTriggered() {
}

void ObjectMgr::onLoadNormalVectorsTriggered() {
}

void ObjectMgr::onColorChangedTriggered() {
    const QColor selectedColor = QColorDialog::getColor(Qt::white, m_parentWidget, "Choose Color");
    if (!selectedColor.isValid()) {
        qDebug() << "Error getting color";
        return;
    }

    m_color = selectedColor;
    redraw();
}
