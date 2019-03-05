#ifndef GALVOWAVEFORMWIDGET_H
#define GALVOWAVEFORMWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>

#include "core/galvoramp.h"

class GalvoWaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GalvoWaveformWidget(GalvoRamp *galvoRamp,
                                 QWidget *parent = nullptr);

signals:

public slots:

private slots:
    void apply();

private:
    void setupUI();

    GalvoRamp *galvoRamp = nullptr;

    QDoubleSpinBox *offsetSpinBox;
    QDoubleSpinBox *amplitudeSpinBox;
    QSpinBox *delaySpinBox;
};

#endif // GALVOWAVEFORMWIDGET_H
