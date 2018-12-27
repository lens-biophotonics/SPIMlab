#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QTabWidget>

#include "cameradisplay.h"
#include "orcaflash.h"

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = 0);

    void setCamera(OrcaFlash *camera);

signals:

public slots:

private:
    void setupUi();

    CameraDisplay *cameraDisplay;
};

#endif // CENTRALWIDGET_H
