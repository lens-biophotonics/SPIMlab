#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include "logger.h"

class LogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogWidget(QWidget *parent = 0);

signals:

public slots:

private:
    QTextEdit *textEdit;

    bool eventFilter(QObject *obj, QEvent *event);
    void logMessage(QString msg, MsgType type);
};

#endif // LOGWIDGET_HLOGWIDGET_H
