#ifndef LASERPAGE_H
#define LASERPAGE_H

#include <QWidget>

class LaserPage : public QWidget
{
    Q_OBJECT
public:
    explicit LaserPage(QWidget *parent = nullptr);

signals:

public slots:

private:
    void setupUI();
};

#endif // LASERPAGE_H
