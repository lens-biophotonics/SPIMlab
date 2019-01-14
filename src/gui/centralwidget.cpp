#include <QHBoxLayout>

#include "centralwidget.h"
#include "cameradisplay.h"
#include "logwidget.h"
#include "controlwidget.h"


CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void CentralWidget::setupUi()
{
    QTabWidget *tabWidget = new QTabWidget();

    LogWidget *logWidget = new LogWidget();
    CameraDisplay *cameraDisplay = new CameraDisplay();

    tabWidget->addTab(cameraDisplay, "Camera display");
    tabWidget->addTab(logWidget, "Messages");

    ControlWidget *controlWidget = new ControlWidget();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(tabWidget);
    layout->addWidget(controlWidget);
    setLayout(layout);
}
