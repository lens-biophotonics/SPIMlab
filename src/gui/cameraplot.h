#ifndef CAMERPLOT_H
#define CAMERPLOT_H

#include <qwt_plot.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_color_map.h>

class CameraPlot : public QwtPlot
{
public:
    CameraPlot(QWidget *parent = nullptr);

    int heightForWidth(int w) const;
    bool hasHeightForWidth() const;

    void setData(const QVector<double> &vec);
    void setInterval(const Qt::Axis axis, const double min, const double max);

    void setZAutoscaleEnabled(bool enable);

private:
    QwtMatrixRasterData *data;
    bool autoscaleZ = true;
};



class ColorMap : public QwtLinearColorMap
{
public:
    ColorMap(QColor from = Qt::black, QColor to = Qt::white);
};

#endif // CAMERPLOT_H
