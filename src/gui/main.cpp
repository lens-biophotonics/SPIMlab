#include <QApplication>
#include <QStyleFactory>

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

    MainWindow w;
    w.show();

    return a.exec();
}
