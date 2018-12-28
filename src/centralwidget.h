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

signals:

public slots:

private:
    void setupUi();

    boost::shared_ptr<CameraDisplay> cameraDisplay;
    boost::shared_ptr<LogWidget> logWidget;
};

#endif // CENTRALWIDGET_H
