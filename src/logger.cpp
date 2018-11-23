#include <QMessageBox>
#include <QApplication>

#include "logger.h"

Logger::Logger(QString name) : name(name)
{

}

void Logger::info(QString msg, MsgType type)
{
    msg.prepend(QString("[%1] ").arg(name));
    QApplication::postEvent(qApp, new MessageEvent(msg,type));
}

void Logger::error(QString errMsg)
{
    info(errMsg, MSG_ERROR);
}

void Logger::critical(QString msg)
{
    error(msg);
    QMessageBox::critical(0, "Error",msg);
}



const QEvent::Type MessageEvent::TYPE = (QEvent::Type)QEvent::registerEventType();

MessageEvent::MessageEvent(QString msg, MsgType type) :
    QEvent(TYPE){
    this->msg = msg;
    this->type = type;
}

MsgType MessageEvent::getType() {
    return type;
}

QString MessageEvent::getMsg() {
    return msg;
}
