#include <stdexcept>
#include <memory>

#include <QString>
#include <QStateMachine>
#include <QSerialPortInfo>
#include <QMutexLocker>

#include "logger.h"
#include "pidevice.h"
#include "pidaisychain.h"

static Logger *logger = getLogger("PIDevice");


#define FUNCNAME(x) # x
#ifdef WITH_HARDWARE
#define CALL_THROW(functionCall) \
    QMutexLocker ml(&mutex); \
    if (!functionCall) { \
        throw std::runtime_error(std::string(FUNCNAME(functionCall)) + ": " + getErrorString().toStdString()); \
    }
#else
#define CALL_THROW(func)
#endif


PIDevice::PIDevice(QObject *parent) : QObject(parent)
{
    setupStateMachine();
}

PIDevice::PIDevice(const QString &verboseName, QObject *parent) :
    PIDevice (parent)
{
    setVerboseName(verboseName);
}

PIDevice::~PIDevice()
{
    close();
}

void PIDevice::connectSerial(const QString &portName, const int baud)
{
    id = PI_ConnectRS232ByDevName(portName.toStdString().c_str(), baud);

    if (id == -1) {
        throw std::runtime_error("PI_ConnectRS232ByDevName failed");
    }

    setPortName(portName);
    setBaud(baud);
    emit connected();
}

void PIDevice::connectDaisyChainSerial(
    const QString &portName, const int deviceNumber, const int baud)
{
    QString pName = QSerialPortInfo(portName).systemLocation();
    id = openDaisyChain(pName, baud)->connectDevice(deviceNumber);

    setPortName(portName);
    setBaud(baud);
    setDeviceNumber(deviceNumber);

    QString msg =
        QString("Connected Daisy Chain device on port %1, device number %2");
    msg = msg.arg(portName).arg(deviceNumber);
    logger->info(msg);
    emit connected();
}

void PIDevice::connectDevice()
{
    if (getPortName().isEmpty()) {
        logger->warning("Cannot connect: port name not specified");
        return;
    }
    if (getDeviceNumber() > 0) {
        connectDaisyChainSerial(getPortName(), getDeviceNumber(), getBaud());
    }
    else {
        connectSerial(getPortName(), getBaud());
    }

    QString axes = getAxisIdentifiers();
    for (int i = 0; i < axes.length(); ++i) {
        stepSizeMap[axes.at(i)] = 0.1;
    }
}

void PIDevice::close()
{
    PI_CloseConnection(id);
    id = -1;
    axisIdentifiers.clear();
    emit disconnected();
}

bool PIDevice::isConnected()
{
#ifdef WITH_HARDWARE
    return PI_IsConnected(id);
#else
    return id != -1;
#endif
}


void PIDevice::move(const QString &axes, const double pos[])
{
    callFunctionWithVectorOfDoubles(&PI_MOV, axes, pos);
}

void PIDevice::stepUp(const QString &axes)
{
    QVector<double> stepSizes;
    stepSizes.reserve(axes.length());
    for (int i = 0; i < axes.length(); ++i) {
        stepSizes.insert(i, stepSizeMap.value(axes.at(i)));
    }
    moveRelative(axes, stepSizes.data());
}

void PIDevice::stepDown(const QString &axes)
{
    QVector<double> stepSizes;
    stepSizes.reserve(axes.length());
    for (int i = 0; i < axes.length(); ++i) {
        stepSizes.insert(i, -stepSizeMap.value(axes.at(i)));
    }
    moveRelative(axes, stepSizes.data());
}

void PIDevice::moveRelative(const QString &axes, const double pos[])
{
    callFunctionWithVectorOfDoubles(&PI_MVR, axes, pos);
}

void PIDevice::setServoEnabled(const QString &axes, const QVector<int> &enable)
{
    if (axes.isEmpty()) {
        return;
    }
#ifndef WITH_HARDWARE
    Q_UNUSED(enable)
#endif
    CALL_THROW(PI_SVO(id, axes.toLatin1(), enable.constData()));
}

/**
 * @brief Set servo-control for all axes.
 * @param enable
 */

void PIDevice::setServoEnabled(bool enable)
{
    QString axes = getAxisIdentifiers();
    setServoEnabled(axes, QVector<int>(axes.length(), enable ? 1 : 0));
}

void PIDevice::halt(const QString &axes)
{
    CALL_THROW(PI_HLT(id, axes.toLatin1()));
}

void PIDevice::setVelocities(const QString &axes, const double vel[])
{
    callFunctionWithVectorOfDoubles(&PI_VEL, axes, vel);
}

QVector<double> PIDevice::getVelocities(const QString &axes)
{
    return *getVectorOfDoubles(&PI_qVEL, axes).get();
}

QVector<double> PIDevice::getTravelRangeLowEnd(const QString &axes)
{
    return *getVectorOfDoubles(&PI_qTMN, axes).get();
}

QVector<double> PIDevice::getTravelRangeHighEnd(const QString &axes)
{
    return *getVectorOfDoubles(&PI_qTMX, axes).get();
}

QVector<double> PIDevice::getCurrentPosition(const QString &axes)
{
    auto ret = getVectorOfDoubles(&PI_qPOS, axes);
    emit newPositions(axes.isEmpty() ? getAxisIdentifiers() : axes, *ret.get());
    return *ret.get();
}

void PIDevice::fastMoveToPositiveLimit(const QString &axes)
{
    if (axes.isEmpty()) {
        return;
    }
    CALL_THROW(PI_FPL(id, axes.toLatin1()));
}

void PIDevice::fastMoveToNegativeLimit(const QString &axes)
{
    if (axes.isEmpty()) {
        return;
    }
    CALL_THROW(PI_FNL(id, axes.toLatin1()));
}

void PIDevice::fastMoveToReferenceSwitch(const QString &axes)
{
    if (axes.isEmpty()) {
        return;
    }
    CALL_THROW(PI_FRF(id, axes.toLatin1()));
}

void PIDevice::loadStages(
    const QString &axes, const QStringList &stages)
{
    if (axes.isEmpty() || stages.isEmpty()) {
        return;
    }
    CALL_THROW(PI_CST(id, axes.toLatin1(), stages.join("\n").toLatin1()));
}

QStringList PIDevice::getStages(const QString &axes, bool stripAxes)
{
#ifndef WITH_HARDWARE
    Q_UNUSED(axes)
#endif
    std::unique_ptr<char[]> buf(new char[1024]);
    CALL_THROW(PI_qCST(id, axes.toLatin1(), buf.get(), 1024));
    QStringList sl = QString(buf.get()).split("\n");
    sl.removeAll(QString(""));
    if (stripAxes) {
        sl.replaceInStrings(QRegExp("\\d+="), "");
    }
    return sl;
}

QStringList PIDevice::getAvailableStageTypes()
{
    std::unique_ptr<char[]> buf(new char[4096]);
#ifdef WITH_HARDWARE
    CALL_THROW(PI_qVST(id, buf.get(), 4096));
#endif
    QStringList sl = QString(buf.get()).replace(" ", "").split("\n");
    sl.removeAll(QString(""));
    return sl;
}

QString PIDevice::getAxisIdentifiers()
{
    if (!axisIdentifiers.isEmpty()) {
        return axisIdentifiers;
    }
    std::unique_ptr<char[]> buf(new char[32]);
    CALL_THROW(PI_qSAI(id, buf.get(), 32));
    axisIdentifiers = QString(buf.get()).replace("\n", "");
    return axisIdentifiers;
}

QVector<int> PIDevice::getReferencedState(QString axes)
{
    int nOfAxes;
    if (axes.isEmpty()) {
        axes = getAxisIdentifiers();
    }
    nOfAxes = getAxisIdentifiers().length();
    QVector<BOOL> vec;
    vec.resize(nOfAxes);
    std::unique_ptr<BOOL[]> buf(new BOOL[static_cast<size_t>(nOfAxes)]);
    CALL_THROW(PI_qFRF(id, axes.toLatin1(), vec.data()));
    return vec;
}

int PIDevice::getError()
{
    return PI_GetError(id);
}

QString PIDevice::getErrorString()
{
    std::unique_ptr<char[]> buf(new char[1024]);
    PI_TranslateError(getError(), buf.get(), 1024);
    return QString(buf.get());
}

QState *PIDevice::connectedState() const
{
    return _connectedState;
}

QState *PIDevice::disconnectedState() const
{
    return _disconnectedState;
}

void PIDevice::setupStateMachine()
{
    _connectedState = new QState();
    _disconnectedState = new QState();

    _connectedState->addTransition(
        this, &PIDevice::disconnected, _disconnectedState);

    _disconnectedState->addTransition(
        this, &PIDevice::connected, _connectedState);

    QStateMachine *sm = new QStateMachine();

    sm->addState(_connectedState);
    sm->addState(_disconnectedState);

    sm->setInitialState(_disconnectedState);
    sm->start();
}

std::unique_ptr<QVector<double>> PIDevice::getVectorOfDoubles(
    const PI_qVectorOfDoubles fp, const QString &axes)
{
    auto vecup = std::make_unique<QVector<double>>();
    const QString *myAxes = &axes;
    QString temp;
    if (axes.isEmpty()) {
        temp = getAxisIdentifiers();
        myAxes = &temp;
    }
    int nOfAxes = myAxes->length();
    vecup.get()->resize(nOfAxes);
    CALL_THROW(fp(id, myAxes->toLatin1(), vecup.get()->data()));
    return vecup;
}

void PIDevice::callFunctionWithVectorOfDoubles(
    PI_vectorOfDoubles fp, const QString &axes, const double values[])
{
    CALL_THROW(fp(id, axes.toLatin1(), values));
}

QString PIDevice::getVerboseName() const
{
    return verboseName;
}

void PIDevice::setVerboseName(const QString &value)
{
    verboseName = value;
}

double PIDevice::getStepSize(const QString &axis) const
{
    return stepSizeMap.value(axis);
}

void PIDevice::setStepSize(const QString &axis, const double value)
{
    if (!stepSizeMap.contains(axis)) {
        return;
    }
    stepSizeMap[axis] = value;
}

QString PIDevice::getPortName() const
{
    return portName;
}

void PIDevice::setPortName(const QString &value)
{
    portName = value;
}

int PIDevice::getBaud() const
{
    return baud;
}

void PIDevice::setBaud(int value)
{
    baud = value;
}

int PIDevice::getDeviceNumber() const
{
    return deviceNumber;
}

void PIDevice::setDeviceNumber(int value)
{
    deviceNumber = value;
}
