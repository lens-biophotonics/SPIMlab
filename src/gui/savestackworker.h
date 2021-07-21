#ifndef SAVESTACKWORKER_H
#define SAVESTACKWORKER_H

#include <QString>

class OrcaFlash;

class SaveStackWorker : public QObject
{
    Q_OBJECT
public:
    explicit SaveStackWorker(OrcaFlash *orca, QObject *parent = nullptr);

    void layOutFileOnDisk();
    double getTimeout() const; // ms
    void setTimeout(double value); // ms
    void setFrameCount(int32_t count);
    size_t getFrameCount() const;
    void setOutputFileName(const QString &fname);
    void setOutputPath(const QString &value);

    void signalTriggerCompletion();

    size_t getReadFrames() const;

    QString rawFileName();
    QString mhdFileName();

    void start();
    void stop();

signals:
    void error(QString msg = "");
    void captureCompleted(bool ok);

private:
    QString timeoutString(double delta, int i);

    bool stopped, triggerCompleted;
    double timeout;
    QString outputFileName;
    QString outputPath;
    size_t frameCount, readFrames;
    OrcaFlash *orca;
};

#endif // SAVESTACKWORKER_H
