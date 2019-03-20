#include <qwt_plot_spectrogram.h>
#include <qwt_scale_engine.h>

#include "cameraplot.h"

#define NROWS 2048
#define NCOLS 2048

CameraPlot::CameraPlot(QWidget *parent) : QwtPlot(parent)
{
    axisScaleEngine(xBottom)->setAttribute(QwtScaleEngine::Floating, true);
    axisScaleEngine(yLeft)->setAttribute(QwtScaleEngine::Floating, true);

    ColorMap *colorMap = new ColorMap();

    QwtPlotSpectrogram *spectrogramPlot = new QwtPlotSpectrogram();
    spectrogramPlot->attach(this);
    spectrogramPlot->setColorMap(colorMap);

    data = new QwtMatrixRasterData();
    data->setInterval(Qt::ZAxis, QwtInterval(0, 65535));
    data->setInterval(Qt::XAxis, QwtInterval(0, NCOLS));
    data->setInterval(Qt::YAxis, QwtInterval(0, NROWS));

    QVector<double> vec = QVector<double>(NROWS * NCOLS, 0.);

    setData(vec);
    spectrogramPlot->setData(data);

    replot();
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
    if (autoscaleZ) {
        double min = std::numeric_limits<double>::infinity();
        double max = -min;
        data->setValueMatrix(vec, NCOLS);
        foreach(const double val, vec){
            if (val > max) {
                max = val;
            }
            if (val < min) {
                min = val;
            }
        }
        setInterval(Qt::ZAxis, min, max);
    }
    replot();
}

void CameraPlot::setInterval(const Qt::Axis axis, const double min,
                             const double max)
{
    data->setInterval(axis, QwtInterval(min, max));
}

void CameraPlot::setZAutoscaleEnabled(bool enable)
{
    autoscaleZ = enable;
}

ColorMap::ColorMap(QColor from, QColor to) : QwtLinearColorMap(from, to)
{
    setColorInterval(from, to);
    setMode(ScaledColors);
}
