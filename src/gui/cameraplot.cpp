#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>

#include <QtDebug>

#include "cameraplot.h"

#include "colormaps.h"

#define NROWS 2048
#define NCOLS 2048


CameraPlot::CameraPlot(QWidget *parent) : QwtPlot(parent)
{
    axisScaleEngine(xBottom)->setAttribute(QwtScaleEngine::Floating, true);
    axisScaleEngine(yLeft)->setAttribute(QwtScaleEngine::Floating, true);

    spectrogramPlot = new QwtPlotSpectrogram();
    spectrogramPlot->attach(this);

    data = new QwtMatrixRasterData();
    data->setInterval(Qt::XAxis, QwtInterval(0, NCOLS));
    data->setInterval(Qt::YAxis, QwtInterval(0, NROWS));

    QVector<double> vec = QVector<double>(NROWS * NCOLS, 0.);
    for (int i = 0; i < vec.size(); ++i) {
        vec[i] = i / 64;
    }

    QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
    rightAxis->setColorBarEnabled(true);
    enableAxis(yRight);

    setData(vec);
    spectrogramPlot->setData(data);

    setColorMap(new GrayScaleColorMap());
    setInterval(Qt::ZAxis, 0, 65535);
}

int CameraPlot::heightForWidth(int w) const
{
    return w;
}

bool CameraPlot::hasHeightForWidth() const
{
    return true;
}

void CameraPlot::setData(const QVector<double> &vec)
{
    min = std::numeric_limits<double>::infinity();
    max = -min;
    data->setValueMatrix(vec, NCOLS);
    for (const double val : vec) {
        if (val > max) {
            max = val;
        }
        if (val < min) {
            min = val;
        }
    }
    if (autoscaleZ) {
        setInterval(Qt::ZAxis, min, max);
    }
    replot();
}

void CameraPlot::setInterval(const Qt::Axis axis, const double min,
                             const double max)
{
    ZInterval = QwtInterval(min, max);
    data->setInterval(axis, ZInterval);
    if (axis == Qt::ZAxis) {
        setAxisScale(QwtPlot::yRight, ZInterval.minValue(), ZInterval.maxValue());
        QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
        rightAxis->setColorMap(ZInterval, copyColorMap(colorMap));
    }
    replot();
}

void CameraPlot::setZAutoscaleEnabled(bool enable)
{
    autoscaleZ = enable;
    if (enable) {
        setInterval(Qt::ZAxis, min, max);
    }
    replot();
}

void CameraPlot::setColorMap(QwtLinearColorMap *value)
{
    colorMap = value;

    spectrogramPlot->setColorMap(colorMap);
    QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
    rightAxis->setColorMap(ZInterval, copyColorMap(colorMap));
    replot();
}
