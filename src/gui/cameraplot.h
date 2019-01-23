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

private:
    QwtMatrixRasterData *data;
};



class ColorMap : public QwtLinearColorMap
{
public:
    ColorMap(QColor from = Qt::black, QColor to = Qt::white);
};

#endif // CAMERPLOT_H
