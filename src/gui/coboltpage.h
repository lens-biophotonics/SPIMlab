#ifndef COBOLTPAGE_H
#define COBOLTPAGE_H

#include <QWidget>

class CoboltPage : public QWidget
{
    Q_OBJECT
public:
    explicit CoboltPage(QWidget *parent = nullptr);

signals:

public slots:

private:
    void setupUI();
};

#endif // COBOLTPAGE_H
