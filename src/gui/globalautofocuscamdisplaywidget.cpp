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
    QwtPlotPicker *leftPicker = new RoiPicker(plot->canvas());
    leftPicker->setStateMachine(new QwtPickerDragRectMachine());
    leftPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier);
    leftPicker->setRubberBand(QwtPicker::RectRubberBand);
    leftPicker->setRubberBandPen(QColor(0xff, 0xaa, 0x00));

    QwtPlotPicker *rightPicker = new RoiPicker(plot->canvas());
    rightPicker->setStateMachine(new QwtPickerDragRectMachine());
    rightPicker->setMousePattern(QwtEventPattern::MouseSelect1,
                                 Qt::LeftButton,
                                 Qt::ShiftModifier | Qt::ControlModifier);
    rightPicker->setRubberBand(QwtPicker::RectRubberBand);
    rightPicker->setRubberBandPen(QColor(0x00, 0xaa, 0x00));

    image1 = new QwtPlotShapeItem();
    image1->setPen(leftPicker->rubberBandPen());
    image1->attach(plot);

    image2 = new QwtPlotShapeItem();
    image2->setPen(rightPicker->rubberBandPen());
    image2->attach(plot);

    connect(leftPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                setImage1(rect);
                emit newImage1(rect);
            });

    connect(rightPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                setImage2(rect);
                emit newImage2(rect);
            });

    menu->addSeparator();
    menu->addAction("Clear left ROI", [=]() {
        image1Item->setShape(QPainterPath());
        plot->replot();
    });
    menu->addAction("Clear right ROI", [=]() {
        image2Item->setShape(QPainterPath());
        plot->replot();
    });
}

void AutofocusCamDisplayWidget::setImage1(QRectF rect)
{
    QPainterPath pp;
    pp.addRect(rect);
    image1Item->setShape(pp);
    plot->replot();
}

void AutofocusCamDisplayWidget::setImage2(QRectF rect)
{
    QPainterPath pp;
    pp.addRect(rect);
    image2Item->setShape(pp);
    plot->replot();
}
