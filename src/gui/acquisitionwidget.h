#ifndef ACQUISITIONWIDGET_H
#define ACQUISITIONWIDGET_H

#include <QWidget>

class AcquisitionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AcquisitionWidget(QWidget *parent = nullptr);

signals:

public slots:

private:
    void setupUI();
};

#endif // ACQUISITIONWIDGET_H
