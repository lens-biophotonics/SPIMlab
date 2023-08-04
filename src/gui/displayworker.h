#ifndef DISPLAYWORKER_H
#define DISPLAYWORKER_H

#include <QThread>

class OrcaFlash;

class DisplayWorker : public QThread
{
    Q_OBJECT
public:
    DisplayWorker(OrcaFlash *orca, QObject *parent = nullptr);
    virtual ~DisplayWorker();

    void setBinning(uint value);

    void setVerticalFlipEnabled(bool enable);

signals:
    void newImage(double *data, size_t n);

protected:
    virtual void run();

private:
    OrcaFlash *orca;
    double *mybufDouble;
    bool running;
    bool flipVertically = false;
    uint binning = 1;
};

#endif // DISPLAYWORKER_H
