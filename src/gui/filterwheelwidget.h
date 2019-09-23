#ifndef FILTERWHEELWIDGET_H
#define FILTERWHEELWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>


class FilterWheel;

class FilterWheelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilterWheelWidget(FilterWheel *fw, QWidget *parent = nullptr);

signals:

private slots:
    void connectDevice();
    void disconnectDevice();
    void refreshValues();

private:
    void setupUI();

    FilterWheel *fw;
    QComboBox *serialPortComboBox;
    QComboBox *filterComboBox;
    QLabel *filterLabel;
    bool motionEnabled = false;
};

#endif // FILTERWHEELWIDGET_H
