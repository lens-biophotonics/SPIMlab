#ifndef AUTOFOCUSCAMDISPLAYWIDGET_H
#define AUTOFOCUSCAMDISPLAYWIDGET_H

#include <qtlab/widgets/cameradisplay.h>

#include <qwt_plot_shapeitem.h>

class AutofocusCamDisplayWidget : public CameraDisplay
{
    Q_OBJECT
public:
    AutofocusCamDisplayWidget(QWidget *parent = nullptr);

    void setUpLeftRoi(QRectF);
    void setUpRightRoi(QRectF rect);
    void setDownLeftRoi(QRectF);
    void setDownRightRoi(QRectF rect);


signals:
    void newUpLeftRoi(QRectF);
    void newUpRightRoi(QRectF);
    void newDownLeftRoi(QRectF);
    void newDownRightRoi(QRectF);

private:
    QwtPlotShapeItem *upLeftRoiItem;
    QwtPlotShapeItem *upRightRoiItem;
    QwtPlotShapeItem *downLeftRoiItem;
    QwtPlotShapeItem *downRightRoiItem;
};

#endif // AUTOFOCUSCAMDISPLAYWIDGET_H
