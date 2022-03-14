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

    void onNewImage(CAlkUSB3::BufferPtr ptr);

signals:

private:
    void setupUi();

    CAlkUSB3::BufferPtr ptr;
    AutofocusCamDisplayWidget *cd;
    PixmapWidget *pmw;
    double *mybufDouble;
};

#endif // AUTOFOCUSWIDGET_H
