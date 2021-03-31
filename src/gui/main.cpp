#include <QApplication>
#include <QStyleFactory>
#include <QSerialPortInfo>

#include <qtlab/core/logger.h>

#include "mainwindow.h"
#include "version.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setOrganizationDomain("lens.unifi.it");
    QCoreApplication::setApplicationName(PROGRAM_NAME);

    QApplication a(argc, argv);

#ifdef FORCE_FUSION_STYLE
    a.setStyle(QStyleFactory::create("fusion"));
#endif

    Logger *logger = getLogger();

    logger->info("Available serial devices:");
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        QString descr = QString("portName = %1, descr=%2, s/n=%3, manufacturer=%4)")
                        .arg(info.portName())
                        .arg(info.description())
                        .arg(info.serialNumber())
                        .arg(info.manufacturer());

        logger->info(descr);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
