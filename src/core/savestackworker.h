#ifndef SAVESTACKWORKER_H
#define SAVESTACKWORKER_H

#include <QObject>

class SaveStackWorker : public QObject
{
    Q_OBJECT
public:
    explicit SaveStackWorker(QObject *parent = nullptr);

    void setFrameCount(uint count);

public slots:
    void saveToFile();

signals:
    void finished();

private:
    bool stopRequested;
    uint frameCount;
};

#endif // SAVESTACKWORKER_H
