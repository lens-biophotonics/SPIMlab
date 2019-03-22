#ifndef CAMERPLOT_H
#define CAMERPLOT_H

#include <qwt_plot.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>


class CameraPlot : public QwtPlot
{
public:
    CameraPlot(QWidget *parent = nullptr);

    void setData(const QVector<double> &vec);
    void setInterval(const Qt::Axis axis, const double min, const double max);

    void setZAutoscaleEnabled(bool enable);

    void setColorMap(QwtLinearColorMap *value);

private:
    QwtMatrixRasterData *data;
    QwtLinearColorMap *colorMap = nullptr;
    QwtPlotSpectrogram *spectrogramPlot = nullptr;
    bool autoscaleZ = true;
    double min;
    double max;
    QwtInterval ZInterval;
};

#endif // CAMERPLOT_H
