#ifndef PICONTROLLERSETTINGSWIDGET_H
#define PICONTROLLERSETTINGSWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>


#include "core/pidevice.h"


class PIControllerSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PIControllerSettingsWidget(
        const QString &title,
        PIDevice *device,
        QWidget *parent = nullptr);

signals:

public slots:

private slots:
    void configureStages();
    void refreshValues();
    void connectDevice();

private:
    void setupUI();
    PIDevice *device;

    QString title;
    QLabel *axisIdentifiersLabel;
    QLabel *referencedStateLabel;
    QLabel *stagesLabel;

    QComboBox *serialPortComboBox;
    QSpinBox *deviceNumberSpinBox;
    QComboBox *baudComboBox;
};


#endif // PICONTROLLERSETTINGSWIDGET_H
