#include <QHBoxLayout>

#include <boost/signals2/deconstruct.hpp>

#include "centralwidget.h"

namespace bs2 = boost::signals2;


CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void CentralWidget::setupUi()
{
    QTabWidget *tabWidget = new QTabWidget();

    logWidget = bs2::deconstruct<LogWidget>(this);
    cameraDisplay = bs2::deconstruct<CameraDisplay>(this);

    tabWidget->addTab(cameraDisplay.get(), "Camera display");
    tabWidget->addTab(logWidget.get(), "Messages");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}
