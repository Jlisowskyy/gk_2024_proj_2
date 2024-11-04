//
// Created by Jlisowskyy on 11/04/24.
//

/* internal includes */
#include "../include/GraphicObjects/DrawingWidget.h"

/* external includes */
#include <QDebug>
#include <format>
#include <QShortcut>

DrawingWidget::DrawingWidget(QWidget *parent) : QGraphicsView(parent),
                                                m_scene(new QGraphicsScene(this)) {
    Q_ASSERT(parent != nullptr);
    setScene(m_scene);
    m_scene->setBackgroundBrush(Qt::white);

    /* resize the scene */
    m_scene->setSceneRect(0,
                          0,
                          SPACE_WIDTH,
                          SPACE_HEIGHT);


    /* disable scrolls visibility */
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* Additional space options */
    setViewportUpdateMode(FullViewportUpdate);
    setRenderHint(QPainter::Antialiasing, true);

    centerOn(static_cast<double>(SPACE_WIDTH) / 2.0, static_cast<double>(SPACE_HEIGHT) / 2.0);
}

DrawingWidget::~DrawingWidget() {
    clearContent();
}

void DrawingWidget::clearContent() const {
    m_scene->clear();
}
