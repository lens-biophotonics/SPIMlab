#ifndef GALVOWAVEFORMWIDGET_H
#define GALVOWAVEFORMWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>

class GalvoWaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GalvoWaveformWidget(int channelNumber,
                                 QWidget *parent = nullptr);

signals:

public slots:

private slots:
    void apply();

private:
    void setupUI();

    int chNumber = 0;

    QDoubleSpinBox *offsetSpinBox;
    QDoubleSpinBox *amplitudeSpinBox;
    QDoubleSpinBox *delaySpinBox;
};

#endif // GALVOWAVEFORMWIDGET_H
