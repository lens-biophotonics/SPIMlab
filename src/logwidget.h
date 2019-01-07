#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QTextEdit>

#include "logmanager.h"


namespace bs2 = boost::signals2;

class LogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogWidget(QWidget *parent = 0);

signals:

public slots:

private:
    QTextEdit *textEdit;

    void logMessage(QString msg, MsgType type);
};

#endif // LOGWIDGET_HLOGWIDGET_H
