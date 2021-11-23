#include "autofocus.h"

#include <qtlab/core/logger.h>

static Logger *logger = getLogger("Autofocus");

Autofocus::Autofocus(QObject *parent)
    : QObject(parent)
    , dev(CAlkUSB3::ICeleraCamera::Create())
{}

void Autofocus::init()
{
    auto stringArray = dev.GetCameraList();

    size_t n = stringArray.Size();

    if (n == 0) {
        throw std::runtime_error("Cannot find Alkeria camera.");
    }

    logger->info(QString("Found %1 Alkeria cameras").arg(n));

    for (size_t i = 0; i < n; ++i) {
        logger->info(QString(stringArray[i]));
    }

    dev.SetCamera(0);            // Open device
    dev.SetPreserveRates(false); // Ask always for the best performances (FPS)

    dev.RawFrameAcquired().SetUserData(this);
    dev.RawFrameAcquired().SetCallbackEx(&Autofocus::onFrameAcquired);
}

void Autofocus::start()
{
    dev.SetEnableImageThread(false); // No image conversion needed
    dev.SetFrameRate(5);

    dev.SetAcquire(true);
}

void Autofocus::stop()
{
    dev.SetAcquire(false);
}

/**
 * @brief This callback is called upon frame acquisition.
 * @param userData A pointer to this class object.
 */
void Autofocus::onFrameAcquired(void *userData)
{
    if (!userData)
        return;

    Autofocus *videoSource = static_cast<Autofocus *>(userData);
    videoSource->processAcquiredFrame();
}

void Autofocus::processAcquiredFrame() {}
