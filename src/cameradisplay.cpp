#include <qwt_plot_spectrogram.h>
#include <qwt_scale_engine.h>

#include "cameradisplay.h"

#define NROWS 2048
#define NCOLS 2048

CameraDisplay::CameraDisplay(QWidget *parent) : QwtPlot(parent)
{
    axisScaleEngine(xBottom)->setAttribute(QwtScaleEngine::Floating, true);
    axisScaleEngine(yLeft)->setAttribute(QwtScaleEngine::Floating, true);

    ColorMap *colorMap = new ColorMap();

    QwtPlotSpectrogram *spectrogramPlot = new QwtPlotSpectrogram();
    spectrogramPlot->attach(this);
    spectrogramPlot->setColorMap(colorMap);

    data = new QwtMatrixRasterData();

    QVector<double> vec = QVector<double>(NROWS * NCOLS, 0.);
    for (int i = 0; i < vec.size(); ++i) {
        double a = rand() / 1e6;
        vec[i] = a;
    }

    setData(vec);
    spectrogramPlot->setData(data);

    replot();
}

int CameraDisplay::heightForWidth(int w) const
{
    return w;
}

bool CameraDisplay::hasHeightForWidth() const
{
    return true;
}

void CameraDisplay::setData(QVector<double> vec)
{
    data->setValueMatrix(vec, NCOLS);
    data->setInterval(Qt::ZAxis, QwtInterval(0, 2000));
    data->setInterval(Qt::XAxis, QwtInterval(0, NCOLS));
    data->setInterval(Qt::YAxis, QwtInterval(0, NROWS));
    replot();
}
