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
    for (int i = 0; i < vec.size(); ++i) {
        double a = rand() / 1e6;
        vec[i] = a;
    }

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

void CameraPlot::setData(QVector<double> &vec)
{
    data->setValueMatrix(vec, NCOLS);
    replot();
}

ColorMap::ColorMap(QColor from, QColor to) : QwtLinearColorMap(from, to)
{
    setColorInterval(from, to);
    setMode(ScaledColors);
}
