#ifndef AUTOFOCUSWIDGET_H
#define AUTOFOCUSWIDGET_H

#include <ICeleraCamera.h>

#include <qtlab/widgets/pixmapwidget.h>

#include <QWidget>

class AutofocusCamDisplayWidget;

class AutofocusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AutofocusWidget(QWidget *parent = nullptr);
    virtual ~AutofocusWidget();

    void onNewImage(QList<CAlkUSB3::BufferPtr> ptr);

signals:

private:
    void setupUi();

    QList<CAlkUSB3::BufferPtr> ptr;
    CameraDisplay *cd1;
    CameraDisplay *cd2;
    PixmapWidget *pmw[4];
    double *mybufDouble1;
    double *mybufDouble2;
};

#endif // AUTOFOCUSWIDGET_H
