#ifndef CAMERAPAGE_H
#define CAMERAPAGE_H

#include <qtlab/hw/pi-widgets/pipositioncontrolwidget.h>

#include <QWidget>

class CameraPage : public QWidget
{
    Q_OBJECT
public:
    explicit CameraPage(QWidget *parent = nullptr);
    virtual ~CameraPage();

signals:

public slots:

private:
    void setupUI();
    void saveSettings();

    PIPositionControlWidget *stageCw;
};

#endif // CAMERAPAGE_H
