#include <QHBoxLayout>
#include <QSettings>

#include "core/spim.h"

#include "centralwidget.h"
#include "cameradisplay.h"
#include "logwidget.h"
#include "settingspage.h"
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
    QWidget *cameraPage = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(new CameraDisplay(spim().getCamera(0)));
    hLayout->addWidget(new CameraDisplay(spim().getCamera(1)));

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout);
    vLayout->addStretch();

    cameraPage->setLayout(vLayout);

    tabWidget = new QTabWidget();

    LogWidget *logWidget = new LogWidget();

    tabWidget->addTab(cameraPage, "Camera display");
    tabWidget->addTab(new SettingsPage(), "Settings");
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
