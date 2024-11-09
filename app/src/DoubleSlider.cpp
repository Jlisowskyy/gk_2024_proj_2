//
// Created by Jlisowskyy on 04/11/24.
//

/* internal includes */
#include "../include/UiObjects/DoubleSlider.h"

/* external includes */
#include <QLabel>
#include <QVBoxLayout>

DoubleSlider::DoubleSlider(const Qt::Orientation orientation, QWidget *parent, const double minValue,
                           const double maxValue,
                           const int precisionPoints, const int startValuePrecision, const char *title,
                           const char *toolTip, const int sliderSize) : QSlider(orientation), m_minValue(minValue),
                                                                        m_maxValue(maxValue) {
    Q_ASSERT(precisionPoints >= 0);
    Q_ASSERT(startValuePrecision >= 0 && startValuePrecision <= precisionPoints);

    setRange(0, precisionPoints);
    setValue(startValuePrecision);
    setToolTip(tr(toolTip));

    auto *pContainer = new QWidget(parent);
    const auto pLayout = new QVBoxLayout(pContainer);
    pLayout->setContentsMargins(2, 2, 2, 2);
    pLayout->setSpacing(1);

    auto *pLabelsContainer = new QWidget(pContainer);
    auto *pLabelsLayout = new QHBoxLayout(pLabelsContainer);
    pLabelsLayout->setContentsMargins(0, 0, 0, 0);
    pLabelsLayout->setSpacing(2);

    setParent(pContainer);

    auto *pLabel = new QLabel(tr(title), pContainer);
    pLabel->setToolTip(tr(toolTip));
    pLabel->setAlignment(Qt::AlignCenter);
    pLayout->addWidget(pLabel);
    pLayout->addWidget(this);
    pLayout->addWidget(pLabelsContainer);

    pContainer->setLayout(pLayout);

    auto *minLabel = new QLabel(QString::number(minValue), pLabelsContainer);
    minLabel->setFixedWidth(minLabel->sizeHint().width());

    pLabel = new QLabel(QString::number(getDoubleValue()), pLabelsContainer);
    pLabel->setAlignment(Qt::AlignCenter);

    auto *maxLabel = new QLabel(QString::number(maxValue), pLabelsContainer);
    maxLabel->setFixedWidth(maxLabel->sizeHint().width());

    pLabelsLayout->addWidget(minLabel);
    pLabelsLayout->addStretch();
    pLabelsLayout->addWidget(pLabel);
    pLabelsLayout->addStretch();
    pLabelsLayout->addWidget(maxLabel);

    connect(this, &DoubleSlider::doubleValueChanged, pLabel, [pLabel](double value) {
        pLabel->setText(QString::number(value));
    });

    pLabelsContainer->setLayout(pLabelsLayout);

    m_container = pContainer;

    if (sliderSize != 0) {
        orientation == Qt::Horizontal ? setFixedWidth(sliderSize) : setFixedHeight(sliderSize);
    }
}

void DoubleSlider::sliderChange(const SliderChange change) {
    QSlider::sliderChange(change);
    if (change == QSlider::SliderValueChange) {
        emit doubleValueChanged(getDoubleValue());
    }
}