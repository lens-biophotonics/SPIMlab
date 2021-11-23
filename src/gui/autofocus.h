#ifndef AUTOFOCUS_H
#define AUTOFOCUS_H

#include <ICeleraCamera.h>

#include <QObject>

class Autofocus : public QObject
{
    Q_OBJECT
public:
    explicit Autofocus(QObject *parent = nullptr);

    void init();

    void start();
    void stop();

signals:

private:
    /**
     * @brief This object controls the camera.
     */
    CAlkUSB3::ICeleraCamera &dev;

    static void onFrameAcquired(void *userData);
    void processAcquiredFrame();
};

#endif // AUTOFOCUS_H
