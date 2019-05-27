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

    void layOutFileOnDisk();
    void setFrameCount(int32_t count);
    void setOutputFileName(const QString &fname);
    void setOutputPath(const QString &value);

    QString rawFileName();
    QString mhdFileName();

signals:
    void error(QString msg = "");

protected:
    virtual void run();

private:
    bool stopRequested;
    QString outputFileName;
    QString outputPath;
    int32_t frameCount;
    OrcaFlash *orca;
};

#endif // SAVESTACKWORKER_H
