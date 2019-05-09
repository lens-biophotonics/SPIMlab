#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>

class ProgressWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressWidget(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // PROGRESSWIDGET_H
