#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QTextEdit>

#include <qtlab/core/logger.h>

class LogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogWidget(QWidget *parent = nullptr);

signals:

public slots:

private:
    QTextEdit *textEdit;

private slots:
    void logMessage(QString msg, const MsgType type);
};

#endif // LOGWIDGET_HLOGWIDGET_H
