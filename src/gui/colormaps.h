#ifndef COLORMAPS_H
#define COLORMAPS_H

#include <qwt_color_map.h>

QwtLinearColorMap *copyColorMap(const QwtLinearColorMap *colorMap);

class GrayScaleColorMap : public QwtLinearColorMap
{
public:
    GrayScaleColorMap();
};

class BlueWhiteColorMap : public QwtLinearColorMap
{
public:
    BlueWhiteColorMap();
};

class HiLowColorMap : public QwtLinearColorMap
{
public:
    HiLowColorMap();
};

class IJLUTColorMap : public QwtLinearColorMap
{
public:
    IJLUTColorMap(QString fname);
};

#endif // COLORMAPS_H
