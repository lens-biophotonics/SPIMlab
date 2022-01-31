#ifndef FILTERSWIDGET_H
#define FILTERSWIDGET_H

#include <QWidget>

class FiltersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FiltersWidget(QWidget *parent = nullptr);

signals:

public slots:

private:
    void setupUI();
};

#endif // FILTERSWIDGET_H
