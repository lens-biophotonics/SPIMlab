#include <QResizeEvent>
#include <QtDebug>

#include <qwt/qwt_plot_layout.h>

#include "aspectratiowidget.h"

AspectRatioWidget::AspectRatioWidget(QWidget *widget, double width, double height,
                                     int paddingW, int paddingH, QWidget *parent) :
    QWidget(parent), wdg(widget), arWidth(width), arHeight(height),
    paddingW(paddingW), paddingH(paddingH)
{
    layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setMargin(0);

    // add spacer, then your widget, then spacer
    layout->addItem(new QSpacerItem(0, 0));
    layout->addWidget(widget);
    layout->addItem(new QSpacerItem(0, 0));
}

void AspectRatioWidget::resizeEvent(QResizeEvent *event)
{
    double thisAspectRatio = static_cast<double>(event->size().width())
                             / event->size().height();
    int widgetStretch, outerStretch;

    double h = height() + paddingH;
    double w = width() + paddingW;

    if (thisAspectRatio > (arWidth / arHeight)) // too wide
    {
        layout->setDirection(QBoxLayout::LeftToRight);
        widgetStretch = static_cast<int>(h * (arWidth / arHeight)); // i.e., my width
        outerStretch = static_cast<int>((w - widgetStretch) / 2 + 0.5);
    }
    else // too tall
    {
        layout->setDirection(QBoxLayout::TopToBottom);
        widgetStretch = static_cast<int>(w * (arHeight / arWidth)); // i.e., my height
        outerStretch = static_cast<int>((h - widgetStretch) / 2 + 0.5);
    }

    layout->setStretch(0, outerStretch);
    layout->setStretch(1, widgetStretch);
    layout->setStretch(2, outerStretch);
}
