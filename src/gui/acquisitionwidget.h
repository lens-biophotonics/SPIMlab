#ifndef ACQUISITIONWIDGET_H
#define ACQUISITIONWIDGET_H

#include <QWidget>
#include <QLineEdit>

class AcquisitionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AcquisitionWidget(QWidget *parent = nullptr);

    QString getRunName();

signals:

public slots:

private:
    void setupUI();
    QLineEdit *runNameLineEdit;
};

#endif // ACQUISITIONWIDGET_H
