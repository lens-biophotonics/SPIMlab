#include "globalautofocuscamdisplaywidget.h"

#include <qtlab/widgets/cameraplot.h>

#include <qwt_picker_machine.h>

class RoiPicker : public QwtPlotPicker
{
public:
    RoiPicker(QWidget *canvas)
        : QwtPlotPicker(canvas)
    {
        setTrackerMode(ActiveOnly);
    }

    virtual QwtText trackerTextF(const QPointF &pos) const
    {
        QColor bg(Qt::white);
        bg.setAlpha(200);

        QwtText text = QString("%1, %2").arg(pos.x()).arg(pos.y());
        text.setBackgroundBrush(QBrush(bg));
        return text;
    }
};

AutofocusCamDisplayWidget::AutofocusCamDisplayWidget(QWidget *parent)
    : CameraDisplay(parent)
{
    QwtPlotPicker *globalPicker = new RoiPicker(plot->canvas());
    globalPicker->setStateMachine(new QwtPickerDragRectMachine());
    globalPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier);
    globalPicker->setRubberBand(QwtPicker::RectRubberBand);
    globalPicker->setRubberBandPen(QColor(0xff, 0xaa, 0x00));

    QPainterPath sharedPath;
  
    image1Item = new QwtPlotShapeItem();
    image1Item->setPen(globalPicker->rubberBandPen());
    image1Item->setShape(sharedPath);
    image1Item->attach(plot);
    
    image2Item = new QwtPlotShapeItem(); 
    image2Item->setShape(sharedPath);
    image2Item->attach(plot);    
    
    connect(globalPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                sharedPath = QPainterPath();
                sharedPath.addRect(rect);
                setImage1(sharedPath);
                emit newImage1(rect);
                setImage2(sharedPath);
                emit newImage2(rect);
            });
    
    menu->addSeparator();
    menu->addAction("Clear image 1", [=]() {
        image1Item->setShape(sharedPath);
        plot->replot();
    });
    menu->addAction("Clear image 2", [=]() {
        image2Item->setShape(sharedPath);
        plot->replot();
    });
}

void AutofocusCamDisplayWidget::setImage1(QPainterPath sharedPath)
{
    QPainterPath pp;
    pp.addRect( sharedPath);
    image1Item->setShape(pp);
    plot->replot();
}

void AutofocusCamDisplayWidget::setImage2(QPainterPath sharedPath)
{
    QPainterPath pp;
    pp.addRect(sharedPath);
    image2Item->setShape(pp);
    plot->replot();
}
