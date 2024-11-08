//
// Created by Jlisowskyy on 11/04/24.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/* External includes */
#include <QApplication>
#include <QMainWindow>

/* Forward declarations */
class DrawingWidget;
class ToolBar;
class StateMgr;

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    // ------------------------------
    // Class protected methods
    // ------------------------------
protected:
    // ------------------------------
    // Class fields
    // ------------------------------

private:
    Ui::MainWindow *m_ui;
    ToolBar *m_toolBar;
    DrawingWidget *m_drawingWidget;
    StateMgr *m_objectMgr;
};

#endif // MAINWINDOW_H
