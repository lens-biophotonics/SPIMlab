#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setOrganizationDomain("lens.unifi.it");
    QCoreApplication::setApplicationName(PROGRAM_NAME);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
