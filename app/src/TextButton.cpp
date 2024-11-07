//
// Created by Jlisowskyy on 11/5/24.
//

/* internal includes */
#include "../include/UiObjects/TextButton.h"
#include "../include/Constants.h"

/* external includes */
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

TextButton::TextButton(QWidget *parent, const char *toolTip, const char *title, const char *icon) : QWidget(parent) {
    setToolTip(tr(toolTip));

    auto pLayout = new QHBoxLayout(this);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setSpacing(0);

    auto pToolButton = new QToolButton(this);
    m_button = new QAction(tr(title), pToolButton);
    m_button->setToolTip(tr(toolTip));
    m_button->setIcon(QIcon(icon));
    m_button->setIconVisibleInMenu(true);
    pToolButton->setDefaultAction(m_button);
    pToolButton->setIconSize(QSize(UI_CONSTANTS::DEFAULT_ICON_SIZE, UI_CONSTANTS::DEFAULT_ICON_SIZE));

    pLayout->addWidget(new QLabel(QString(tr(title)) + QString(tr(":")), this));
    pLayout->addStretch();

    pLayout->addWidget(pToolButton);
}
