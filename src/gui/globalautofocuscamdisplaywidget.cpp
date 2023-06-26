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

    leftRoiItem = new QwtPlotShapeItem();
    leftRoiItem->setPen(leftPicker->rubberBandPen());
    leftRoiItem->attach(plot);

    rightRoiItem = new QwtPlotShapeItem();
    rightRoiItem->setPen(rightPicker->rubberBandPen());
    rightRoiItem->attach(plot);

    connect(leftPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                setLeftRoi(rect);
                emit newLeftRoi(rect);
            });

    connect(rightPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                setRightRoi(rect);
                emit newRightRoi(rect);
            });

    menu->addSeparator();
    menu->addAction("Clear left ROI", [=]() {
        leftRoiItem->setShape(QPainterPath());
        plot->replot();
    });
    menu->addAction("Clear right ROI", [=]() {
        rightRoiItem->setShape(QPainterPath());
        plot->replot();
    });
}

void AutofocusCamDisplayWidget::setLeftRoi(QRectF rect)
{
    QPainterPath pp;
    pp.addRect(rect);
    leftRoiItem->setShape(pp);
    plot->replot();
}

void AutofocusCamDisplayWidget::setRightRoi(QRectF rect)
{
    QPainterPath pp;
    pp.addRect(rect);
    rightRoiItem->setShape(pp);
    plot->replot();
}
