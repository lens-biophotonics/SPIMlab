#include <QHBoxLayout>
#include <QSettings>

#include "core/spim.h"

#include "camerapage.h"
#include "settingspage.h"
#include "logwidget.h"
#include "controlwidget.h"

#include "centralwidget.h"

CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadSettings();
}

CentralWidget::~CentralWidget()
{
    saveSettings();
}

void CentralWidget::setupUi()
{
    tabWidget = new QTabWidget();

    tabWidget->addTab(new CameraPage(), "Cameras");
    tabWidget->addTab(new SettingsPage(), "Settings");
    tabWidget->addTab(new LogWidget(), "Messages");

    ControlWidget *controlWidget = new ControlWidget();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(tabWidget);
    layout->addWidget(controlWidget);
    setLayout(layout);
}

void CentralWidget::saveSettings() const
{
    QSettings settings;

    settings.beginGroup("CentralWidget");
    settings.setValue("tabWidget_currentIndex", tabWidget->currentIndex());
    settings.endGroup();
}

void CentralWidget::loadSettings()
{
    QSettings settings;
    settings.beginGroup("CentralWidget");
    tabWidget->setCurrentIndex(
        settings.value("tabWidget_currentIndex").toInt());
    settings.endGroup();
}
