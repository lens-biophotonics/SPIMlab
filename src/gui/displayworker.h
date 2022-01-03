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

signals:
    void newImage(double *data, size_t n);

protected:
    virtual void run();

private:
    OrcaFlash *orca;
    double *mybufDouble;
    bool running;
    uint binning = 1;
};

#endif // DISPLAYWORKER_H
