#ifndef COBOLTWIDGET_H
#define COBOLTWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>


class Cobolt;

class CoboltWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CoboltWidget(Cobolt *cobolt, QWidget *parent = nullptr);

signals:

private slots:
    void connectDevice();
    void refreshValues();

private:
    void setupUI();

    Cobolt *cobolt;
    QComboBox *serialPortComboBox;
    QLabel *powerLabel;
};

#endif // COBOLTWIDGET_H
