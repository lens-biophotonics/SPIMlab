#ifndef AUTOFOCUSCAMDISPLAYWIDGET_H
#define AUTOFOCUSCAMDISPLAYWIDGET_H

#include <qtlab/widgets/cameradisplay.h>

#include <qwt_plot_shapeitem.h>

class AutofocusCamDisplayWidget : public CameraDisplay
{
    Q_OBJECT
public:
    AutofocusCamDisplayWidget(QWidget *parent = nullptr);

    void setLeftRoi(QRectF);
    void setRightRoi(QRectF rect);

signals:
    void newLeftRoi(QRectF);
    void newRightRoi(QRectF);

private:
    QwtPlotShapeItem *leftRoiItem;
    QwtPlotShapeItem *rightRoiItem;
};

#endif // AUTOFOCUSCAMDISPLAYWIDGET_H
