#include <QHBoxLayout>

#include "centralwidget.h"
#include "logwidget.h"

CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void CentralWidget::setupUi()
{
    QTabWidget *tabWidget = new QTabWidget();

    LogWidget *logWidget = new LogWidget();

    cameraDisplay = new CameraDisplay(this);

    tabWidget->addTab(cameraDisplay, "Camera display");
    tabWidget->addTab(logWidget, "Messages");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}

void CentralWidget::setCamera(OrcaFlash *camera)
{
    cameraDisplay->setCamera(camera);
}
