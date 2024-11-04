//
// Created by Jlisowskyy on 11/04/24.
//

#ifndef APP_DRAWINGWIDGET_H
#define APP_DRAWINGWIDGET_H

/* external includes */
#include <QObject>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QLabel>
#include <QGraphicsItem>

/* internal includes */
#include "../Constants.h"

/* Forward declaration */

class DrawingWidget : public QGraphicsView {
Q_OBJECT

    // ------------------------------
    // Class constants
    // ------------------------------
public:
    static constexpr int SPACE_WIDTH = 5000;
    static constexpr int SPACE_HEIGHT = 5000;

    // ------------------------------
    // Class creation
    // ------------------------------

    explicit DrawingWidget(QWidget *parent);

    ~DrawingWidget() override;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // ------------------------------
    // Class slots
    // ------------------------------
public slots:

    void clearContent() const;

    // ------------------------------
    // Class signals
    // ------------------------------
signals:


    // ------------------------------
    // Protected methods
    // ------------------------------
protected:


    // ------------------------------
    // Private methods
    // ------------------------------

private:
    // ------------------------------
    // Class fields
    // ------------------------------

    QGraphicsScene *m_scene{};
};


#endif //APP_DRAWINGWIDGET_H
