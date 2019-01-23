#include <QHBoxLayout>
#include <QSettings>

#include "centralwidget.h"
#include "cameradisplay.h"
#include "logwidget.h"
#include "settingswidget.h"
#include "controlwidget.h"


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

    LogWidget *logWidget = new LogWidget();
    CameraDisplay *cameraDisplay = new CameraDisplay();
    SettingsWidget *settingsWidget = new SettingsWidget();

    tabWidget->addTab(cameraDisplay, "Camera display");
    tabWidget->addTab(settingsWidget, "Settings");
    tabWidget->addTab(logWidget, "Messages");

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
