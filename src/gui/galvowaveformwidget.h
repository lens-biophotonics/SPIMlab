#ifndef GALVOWAVEFORMWIDGET_H
#define GALVOWAVEFORMWIDGET_H

#include <QDoubleSpinBox>
#include <QWidget>

class GalvoWaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GalvoWaveformWidget(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // GALVOWAVEFORMWIDGET_H
