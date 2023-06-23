#include "autofocuscamdisplaywidget.h"

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
    QwtPlotPicker *upLeftPicker = new RoiPicker(plot->canvas());
    upLeftPicker->setStateMachine(new QwtPickerDragRectMachine());
    upLeftPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier);
    upLeftPicker->setRubberBand(QwtPicker::RectRubberBand);
    upLeftPicker->setRubberBandPen(QColor(0xff, 0xaa, 0x00));

    QwtPlotPicker *upRightPicker = new RoiPicker(plot->canvas());
    upRightPicker->setStateMachine(new QwtPickerDragRectMachine());
    upRightPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier | Qt::ControlModifier);
    upRightPicker->setRubberBand(QwtPicker::RectRubberBand);
    upRightPicker->setRubberBandPen(QColor(0x00, 0xaa, 0x00));

    QwtPlotPicker *downLeftPicker = new RoiPicker(plot->canvas());
    downLeftPicker->setStateMachine(new QwtPickerDragRectMachine());
    downLeftPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier);
    downLeftPicker->setRubberBand(QwtPicker::RectRubberBand);
    downLeftPicker->setRubberBandPen(QColor(0xff, 0xaa, 0x00));

    QwtPlotPicker *downRightPicker = new RoiPicker(plot->canvas());
    downRightPicker->setStateMachine(new QwtPickerDragRectMachine());
    downRightPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier | Qt::ControlModifier);
    downRightPicker->setRubberBand(QwtPicker::RectRubberBand);
    downRightPicker->setRubberBandPen(QColor(0x00, 0xaa, 0x00));

    upLeftRoiItem = new QwtPlotShapeItem();
    upLeftRoiItem->setPen(upLeftPicker->rubberBandPen());
    upLeftRoiItem->attach(plot);

    upRightRoiItem = new QwtPlotShapeItem();
    upRightRoiItem->setPen(upRightPicker->rubberBandPen());
    upRightRoiItem->attach(plot);

    downLeftRoiItem = new QwtPlotShapeItem();
    downLeftRoiItem->setPen(downLeftPicker->rubberBandPen());
    downLeftRoiItem->attach(plot);

    downRightRoiItem = new QwtPlotShapeItem();
    downRightRoiItem->setPen(downRightPicker->rubberBandPen());
    downRightRoiItem->attach(plot);

    connect(upLeftPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                setUpLeftRoi(rect);
                emit newUpLeftRoi(rect);
            });

    connect(upRightPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                setUpRightRoi(rect);
                emit newUpRightRoi(rect);
            });

    connect(downLeftPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                setDownLeftRoi(rect);
                emit newDownLeftRoi(rect);
            });

    connect(downRightPicker,
            qOverload<const QRectF &>(&QwtPlotPicker::selected),
            [=](const QRectF &rect) {
                setDownRightRoi(rect);
                emit newDownRightRoi(rect);
            });
    

    menu->addSeparator();
    menu->addAction("Clear upper left ROI", [=]() {
        upLeftRoiItem->setShape(QPainterPath());
        plot->replot();
    });
    menu->addAction("Clear upper right ROI", [=]() {
        upRightRoiItem->setShape(QPainterPath());
        plot->replot();
    });
    menu->addAction("Clear buttom left ROI", [=]() {
        downLeftRoiItem->setShape(QPainterPath());
        plot->replot();
    });
    menu->addAction("Clear buttom right ROI", [=]() {
        downRightRoiItem->setShape(QPainterPath());
        plot->replot();
    });
}

void AutofocusCamDisplayWidget::setUpLeftRoi(QRectF rect)
{
    QPainterPath pp;
    pp.addRect(rect);
    upLeftRoiItem->setShape(pp);
    plot->replot();
}

void AutofocusCamDisplayWidget::setUpRightRoi(QRectF rect)
{
    QPainterPath pp;
    pp.addRect(rect);
    upRightRoiItem->setShape(pp);
    plot->replot();
}

void AutofocusCamDisplayWidget::setDownLeftRoi(QRectF rect)
{
    QPainterPath pp;
    pp.addRect(rect);
    downLeftRoiItem->setShape(pp);
    plot->replot();
}

void AutofocusCamDisplayWidget::setDownRightRoi(QRectF rect)
{
    QPainterPath pp;
    pp.addRect(rect);
    downRightRoiItem->setShape(pp);
    plot->replot();
}
