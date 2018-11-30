#ifndef ORCAFLASH_H
#define ORCAFLASH_H

#include <QObject>

namespace HAMAMATSU {
#ifdef DCAMAPI_HEADERS
#include <dcamapi.h>
#endif
}

int init_dcam();

class OrcaFlash : public QObject
{
    Q_OBJECT
public:
    explicit OrcaFlash(QObject *parent = nullptr);
    virtual ~OrcaFlash();
    bool open(int index);
    bool close();

    bool startFreeRun();
    bool stop();

    bool copyLastFrame(void *buf, size_t n);

    QString getLastError();

signals:

public slots:

private:
#ifdef DCAMAPI_HEADERS
    HAMAMATSU::HDCAM h;
#endif
    int nCamera;

    void logLastError(QString label = "");
};

#endif // ORCAFLASH_H
