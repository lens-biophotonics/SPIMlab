#ifndef CAMERPLOT_H
#define CAMERPLOT_H

#include <qwt_plot.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_color_map.h>

class CameraPlot : public QwtPlot
{
public:
    CameraPlot(QWidget *parent = NULL);

    int heightForWidth(int w) const;
    bool hasHeightForWidth() const;

    void setData(QVector<double> vec);

private:
    QwtMatrixRasterData *data;
};



class ColorMap : public QwtLinearColorMap
{
public:
    ColorMap(QColor from = Qt::black, QColor to = Qt::white) : QwtLinearColorMap(from, to)
    {
        setColorInterval(from, to);
        setMode(ScaledColors);
    }
};

#endif // CAMERPLOT_H
