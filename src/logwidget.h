#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QTextEdit>

#include <boost/shared_ptr.hpp>
#include <boost/signals2/deconstruct.hpp>

#include "logmanager.h"


namespace bs2 = boost::signals2;

class LogWidget : public QWidget
{
    Q_OBJECT
public:

    template<typename T> friend
    void adl_postconstruct(const boost::shared_ptr<T> &sp, LogWidget *)
    {
        LogManager::getInstance().newLogMessage.connect(
            newLogMsg_t::slot_type(
                &LogWidget::logMessage, sp.get(), _1, _2).track(sp));
    }


signals:

public slots:

private:
    QTextEdit *textEdit;

    friend class bs2::deconstruct_access;
    explicit LogWidget(QWidget *parent = 0);
    void logMessage(QString msg, MsgType type);
};

#endif // LOGWIDGET_HLOGWIDGET_H
