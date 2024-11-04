//
// Created by Jlisowskyy on 11/04/24.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/* External includes */
#include <QApplication>
#include <QMainWindow>
#include <QLabel>

/* Internal includes */
#include "ManagingObjects/ToolBar.h"
#include "GraphicObjects/DrawingWidget.h"

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

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

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
};

#endif // MAINWINDOW_H
