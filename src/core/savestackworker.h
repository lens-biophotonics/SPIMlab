#ifndef SAVESTACKWORKER_H
#define SAVESTACKWORKER_H

#include <QThread>
#include <QString>

class OrcaFlash;

class SaveStackWorker : public QThread
{
    Q_OBJECT
public:
    explicit SaveStackWorker(OrcaFlash *orca, QObject *parent = nullptr);

    void setFrameCount(uint count);
    void setOutputFileName(const QString &fname);

signals:
    void error(QString msg = "");

protected:
    virtual void run();

private:
    bool stopRequested;
    QString outputFileName;
    uint frameCount;
    OrcaFlash *orca;
};

#endif // SAVESTACKWORKER_H
