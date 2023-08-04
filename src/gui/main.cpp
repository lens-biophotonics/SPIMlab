#include "mainwindow.h"
#include "settings.h"
#include "spim.h"
#include "version.h"

#include <ICeleraCamera.h>

#include <qtlab/core/logger.h>
#include <qtlab/core/logmanager.h>

#include <QApplication>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QStyleFactory>

static Logger *logger = getLogger("Main");

class MyApplication : public QApplication
{
public:
    MyApplication(int &argc, char **argv)
        : QApplication(argc, argv){};

    bool notify(QObject *receiver, QEvent *event)
    {
        bool done = true;
        try {
            done = QApplication::notify(receiver, event);
        } catch (CAlkUSB3::Exception e) {
            logger->critical(e.Message());
            emit spim().error(e.Message());
        } catch (std::runtime_error e) {
            logger->critical(e.what());
            emit spim().error(e.what());
        }
        return done;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setOrganizationDomain("lens.unifi.it");
    QCoreApplication::setApplicationName(PROGRAM_NAME);

    MyApplication a(argc, argv);
    setlocale(LC_NUMERIC, "C");      // needed by PI_GCS library, otherwise it doesn't parse doubles
                                     // in the response strings correctly
    QLocale::setDefault(QLocale::C); // this overrides the system settings: e.g. it allows doubles
                                     // in QSpinBoxes to be entered with dot instead of comma

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

    settings(); // force loading of settings

    MainWindow w;
    w.show();
    QMetaObject::invokeMethod(&w, "restoreWidgets", Qt::QueuedConnection);

    return a.exec();
}
