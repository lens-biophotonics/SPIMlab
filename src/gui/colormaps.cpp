#include <QFile>
#include <QDataStream>
#include <QRegularExpression>

#include "colormaps.h"

QwtLinearColorMap *copyColorMap(const QwtLinearColorMap *colorMap)
{
    if (!colorMap) {
        return nullptr;
    }
    QwtInterval intv(0., 1.);
    QwtLinearColorMap *cm = new QwtLinearColorMap();
    cm->setColorInterval(colorMap->color1(), colorMap->color2());
    for (const double stop : colorMap->colorStops()) {
        cm->addColorStop(stop, colorMap->color(intv, stop));
    }
    cm->setMode(colorMap->mode());
    return cm;
}


GrayScaleColorMap::GrayScaleColorMap() :
    QwtLinearColorMap (Qt::black, Qt::white)
{
}


BlueWhiteColorMap::BlueWhiteColorMap() :
    QwtLinearColorMap (Qt::black, Qt::white)
{
    addColorStop(0.5, Qt::blue);
}

HiLowColorMap::HiLowColorMap() :
    QwtLinearColorMap (Qt::blue, Qt::red)
{
    addColorStop(1. / 256, Qt::black);
    addColorStop(1 - 1. / 256, Qt::white);
}

IJLUTColorMap::IJLUTColorMap(QString fname) :
    QwtLinearColorMap ()
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly))
        return;
    quint16 ncolors = 0;

    QByteArray reds;
    QByteArray greens;
    QByteArray blues;
    QList<QColor> colors;

    bool ok = false;

    qint64 length = file.size();

    if (length > 768) {
        QDataStream in(&file);
        QByteArray id = file.read(4);
        if (id.compare("ICOL") == 0) {  // (NIH Image LUT Header)
            quint16 dummy;
            in >> dummy;
            in >> ncolors;
            file.seek(0x20);
            ok = true;
        }
    }
    else if (length == 768 || length == 970) {
        ncolors = 256;
        file.seek(0);
        ok = true;
    }

    if (ok) {
        reds = file.read(ncolors);
        greens = file.read(ncolors);
        blues = file.read(ncolors);
    }

    if (!ok) {  // Read text LUT
        file.seek(0);
        QTextStream in(&file);
        QStringList allLines = in.readAll().split('\n');
        int nOfLines = allLines.size();
        int c = nOfLines > 256 ? 1 : 0;
        for (; c < nOfLines; ++c) {
            QStringList sl = allLines.at(c).split(QRegularExpression("[\t ]+"));
            if (sl.size() < 3) {
                continue;
            }
            int i = 0;
            if (sl.size() == 4) {
                i = 1;
            }
            reds.append(static_cast<char>(sl.at(i++).toShort()));
            greens.append(static_cast<char>(sl.at(i++).toShort()));
            blues.append(static_cast<char>(sl.at(i++).toShort()));
            ncolors++;
        }
    }

    for (quint16 i = 0; i < ncolors; ++i) {
        colors << QColor(static_cast<quint8>(reds[i]),
                         static_cast<quint8>(greens[i]),
                         static_cast<quint8>(blues[i]));
    }

    if (colors.length() < 2) {
        return;
    }

    setColorInterval(colors.first(), colors.last());

    colors.pop_back();
    colors.pop_front();

    double stepSize = 1. / (colors.size() + 1);
    int i = 1;

    for (const QColor &color : colors) {
        addColorStop(stepSize * i++, color);
    }
}
