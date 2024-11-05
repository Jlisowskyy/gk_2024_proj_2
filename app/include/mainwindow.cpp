//
// Created by Jlisowskyy on 11/04/24.
//


/* internal includes */
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "./GraphicObjects/DrawingWidget.h"
#include "./ManagingObjects/ObjectMgr.h"
#include "./ManagingObjects/ToolBar.h"

/* external includes */
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          m_ui(new Ui::MainWindow),
          m_toolBar(new ToolBar(this)),
          m_drawingWidget(new DrawingWidget(this)),
          m_objectMgr(new ObjectMgr(this, this, m_drawingWidget)) {
    setMinimumSize(600, 900);
    setWindowTitle("Bezier 3D drawer");

    m_ui->setupUi(this);
    m_toolBar->setupToolBar(m_ui->toolBar, m_drawingWidget);
    m_objectMgr->connectToToolBar(m_toolBar);

    m_ui->verticalLayout->addWidget(m_drawingWidget);

    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    frame->setLineWidth(3);
    frame->setStyleSheet("background-color: rgba(0, 0, 0, 0);");
    m_ui->verticalLayout->addWidget(frame);

    /* window connects */
    connect(m_ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    m_objectMgr->loadDefaultSettings();
}


MainWindow::~MainWindow() {
    delete m_ui;
}
