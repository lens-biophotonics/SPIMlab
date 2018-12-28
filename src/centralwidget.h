#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QTabWidget>

#include <boost/shared_ptr.hpp>

#include "cameradisplay.h"
#include "orcaflash.h"
#include "logwidget.h"


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
    boost::shared_ptr<LogWidget> logWidget;
};

#endif // CENTRALWIDGET_H
