#ifndef GALVOWAVEFORMWIDGET_H
#define GALVOWAVEFORMWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>

class GalvoWaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GalvoWaveformWidget(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // GALVOWAVEFORMWIDGET_H
