//
// Created by Jlisowskyy on 08/10/24.
//

#ifndef APP_TOOLBAR_H
#define APP_TOOLBAR_H

/* internal includes */

/* external includes */
#include <QObject>
#include <QToolBar>
#include <QGraphicsItem>

/* Forward declaration */
class DrawingWidget;

class ToolBar : public QObject {
Q_OBJECT

    // ------------------------------
    // Class creation
    // ------------------------------

public:
    explicit ToolBar(QObject *parent);

    ~ToolBar() override;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void setupToolBar(QToolBar *toolBar, DrawingWidget *drawingWidget);

    // ------------------------------
    // Class slots
    // ------------------------------
public slots:

    // ------------------------------
    // Class private methods
    // ------------------------------
private:
    void _addSeparator();

    void _addToolbarLiteral(const char *strLiteral);

    QAction *_addButtonToToolbar(const char *name, const char *imgPath, const char *toolTip);

// ------------------------------
// Public fields
// ------------------------------
public:

protected:
    // ------------------------------
    // Class fields
    // ------------------------------

    QToolBar *m_toolBar{};
    DrawingWidget *m_drawingWidget{};
};


#endif //APP_TOOLBAR_H
