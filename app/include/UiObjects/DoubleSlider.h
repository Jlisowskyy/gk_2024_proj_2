//
// Created by Jlisowskyy on 04/11/24.
//

#ifndef APP_DOUBLESLIDER_H
#define APP_DOUBLESLIDER_H

/* internal includes */

/* external includes */
#include <QSlider>
#include "../Constants.h"

class DoubleSlider : public QSlider {
Q_OBJECT

    // ------------------------------
    // Class creation
    // ------------------------------

public:

    explicit DoubleSlider(Qt::Orientation orientation, QWidget *parent, double minValue, double maxValue,
                          int precisionPoints, int startValuePrecision, const char *title,
                          const char *toolTip, int sliderSize = UI_CONSTANTS::DEFAULT_SLIDER_SIZE);

    ~DoubleSlider() override = default;

    // ------------------------------
    // Class interaction
    // ------------------------------
public:

    [[nodiscard]] double getDoubleValue() const {
        return m_minValue + (m_maxValue - m_minValue) * (value() / (double) maximum());
    }

    QWidget *getContainer() const {
        return m_container;
    }

signals:

    void doubleValueChanged(double value);

    // ------------------------------
    // Overridden methods
    // ------------------------------
protected:

    void sliderChange(SliderChange change) override;

// ------------------------------
    // Class fields
    // ------------------------------

    double m_minValue;
    double m_maxValue;

    QWidget *m_container;
};


#endif //APP_DOUBLESLIDER_H
