#include <QHBoxLayout>
#include <QSettings>
#include <QPushButton>

#include <qtlab/widgets/logwidget.h>
#include <qtlab/widgets/picontrollersettingswidget.h>

#include "spim.h"

#include "camerapage.h"
#include "settingspage.h"
#include "laserpage.h"

#include "centralwidget.h"

class StagePage : public QWidget
{
public:
    explicit StagePage(QWidget *parent = nullptr) : QWidget(parent)
    {
        QHBoxLayout *piHLayout = new QHBoxLayout();
        QHBoxLayout *piHLayout2 = new QHBoxLayout();
        for (int i = 0; i < 3; ++i) {
            piHLayout->addWidget(
                new PIControllerSettingsWidget(spim().getPIDevice(i)));
        }
        for (int i = 3; i < 5; ++i) {
            piHLayout2->addWidget(
                new PIControllerSettingsWidget(spim().getPIDevice(i)));
        }

        piHLayout->addStretch();
        piHLayout2->addStretch();

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addLayout(piHLayout);
        vLayout->addLayout(piHLayout2);
        vLayout->addStretch();
        setLayout(vLayout);
    }
};


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
    tabWidget->addTab(new LaserPage(), "Lasers");
    tabWidget->addTab(new StagePage(), "Stages");
    tabWidget->addTab(new SettingsPage(), "Settings");
    tabWidget->addTab(new LogWidget(), "Messages");

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(tabWidget);
    setLayout(vLayout);
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
