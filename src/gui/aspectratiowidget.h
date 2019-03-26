#ifndef ASPECTRATIOWIDGET_H
#define ASPECTRATIOWIDGET_H

#include <QWidget>
#include <QHBoxLayout>

#include <qwt_plot.h>


class AspectRatioWidget : public QWidget
{
    Q_OBJECT
public:
    AspectRatioWidget(QWidget *widget, double width, double height,
                      int paddingW = 0, int paddingH = 0,
                      QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event);

private:
    QWidget *wdg;
    QBoxLayout *layout;
    double arWidth; // aspect ratio width
    double arHeight; // aspect ratio height
    int paddingW;
    int paddingH;
};

#endif // ASPECTRATIOWIDGET_H
