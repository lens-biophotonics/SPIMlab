#include "settings.h"

#include "cameratrigger.h"
#include "galvoramp.h"
#include "spim.h"
#include "tasks.h"

#ifdef MASTER_SPIM

#include <qtlab/hw/pi/pidevice.h>
#include <qtlab/hw/serial/AA_MPDSnCxx.h>
#include <qtlab/hw/serial/cobolt.h>
#include <qtlab/hw/serial/filterwheel.h>
#include <qtlab/hw/serial/serialport.h>

#include <QSerialPortInfo>
#endif

#include <memory>

#include <QDir>
#include <QSettings>

#define SET_VALUE(group, key, default_val) setValue(group, key, settings.value(key, default_val))

#define SETTINGSGROUP_COBOLT(n) QString("Cobolt_%1").arg(n)
#define SETTINGSGROUP_ACQUISITION "Acquisition"
#define SETTINGSGROUP_AOTF(n) QString("AOTF_%1").arg(n)
#define SETTINGSGROUP_CAMTRIG "CameraTrigger"
#define SETTINGSGROUP_GRAMP "GalvoRamp"

#define SETTING_PULSE_TERMS "pulseTerms"
#define SETTING_BLANKING_TERMS "blankingTerms"

#define SETTING_SCANVELOCITY "scanVelocity"

#define SETTING_FROM "from"
#define SETTING_TO "to"
#define SETTING_STEP "step"
#define SETTING_MOSAIC_ENABLED "mosaicEnabled"

#define SETTING_BAUD "baud"
#define SETTING_DEVICENUMBER "deviceNumber"
#define SETTING_SERIALNUMBER "serialNumber"
#define SETTING_PORTNAME "portName"

#define SETTING_PHYSCHANS "physicalChannels"
#define SETTING_WFPARAMS "waveformParams"

#define SETTING_TRIGGER_TERM "triggerTerm"

#define SETTING_EXPTIME "exposureTime"
#define SETTING_RUN_NAME "runName"
#define SETTING_BINNING "binning"

#define SETTING_TURN_OFF_LASERS_AT_END_OF_ACQUISITION "turnOffLasersAtEndOfAcquisition"

Settings::Settings()
{
    loadSettings();
}

Settings::~Settings() {}

QVariant Settings::value(const QString &group, const QString &key) const
{
    if (!map.contains(group) || !map[group]->contains(key)) {
        return QVariant();
    }
    return map[group]->value(key);
}

void Settings::setValue(const QString &group, const QString &key, const QVariant val)
{
    if (!map.contains(group)) {
        map.insert(group, new SettingsMap());
    }
    map.value(group)->insert(key, val);
}

void Settings::loadSettings()
{
    map.clear();

    QSettings settings;
    QString group;

    group = SETTINGSGROUP_OTHERSETTINGS;
    settings.beginGroup(group);

    SET_VALUE(group, SETTING_LUTPATH, "/opt/Fiji.app/luts/");
    SET_VALUE(group, SETTING_SCANVELOCITY, 2.0);
    QStringList camOutputPath;
    camOutputPath << "/mnt/dualspim"
                  << "/mnt/dualspim";
    SET_VALUE(group, SETTING_CAM_OUTPUT_PATH_LIST, camOutputPath);
    SET_VALUE(group, SETTING_TURN_OFF_LASERS_AT_END_OF_ACQUISITION, false);

    settings.endGroup();

#ifdef MASTER_SPIM
    QStringList groups;
    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        groups << SETTINGSGROUP_AXIS(i);
    }

    for (const QString &group : groups) {
        settings.beginGroup(group);

        QStringList sl;
        sl << SETTING_BAUD << SETTING_DEVICENUMBER << SETTING_SERIALNUMBER << SETTING_PORTNAME
           << SETTING_POS << SETTING_FROM << SETTING_TO << SETTING_STEP;

        for (const QString &s : sl) {
            SET_VALUE(group, s, QVariant());
        }

        SET_VALUE(group, SETTING_STEPSIZE, 0.1);
        SET_VALUE(group, SETTING_VELOCITY, 1.);
        SET_VALUE(group, SETTING_MOSAIC_ENABLED, false);

        settings.endGroup();
    }

    group = SETTINGSGROUP_GRAMP;
    settings.beginGroup(group);

    SET_VALUE(group, SETTING_PHYSCHANS, QStringList({"Dev1/ao2", "Dev1/ao3"}));
    SET_VALUE(group, SETTING_WFPARAMS, QList<QVariant>({0.0, 4.0, 0.0, 0.95, 0.0, 4.0, 0.0, 0.95}));
    settings.endGroup();

    group = SETTINGSGROUP_CAMTRIG;
    settings.beginGroup(group);

    SET_VALUE(group, SETTING_PULSE_TERMS, QStringList({"/Dev1/PFI0", "/Dev1/PFI1"}));
    SET_VALUE(group, SETTING_BLANKING_TERMS, QStringList({"/Dev1/PFI2", "/Dev1/PFI3"}));
    SET_VALUE(group, SETTING_TRIGGER_TERM, "/Dev1/PFI4");

    settings.endGroup();

    group = SETTINGSGROUP_ACQUISITION;
    settings.beginGroup(group);

    SET_VALUE(group, SETTING_EXPTIME, 0.15);
    SET_VALUE(group, SETTING_RUN_NAME, QString());
    SET_VALUE(group, SETTING_BINNING, 1);

    settings.endGroup();

    groups.clear();
    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        groups << SETTINGSGROUP_COBOLT(i);
    }
    for (const QString &group : groups) {
        settings.beginGroup(group);

        SET_VALUE(group, SETTING_PORTNAME, QString());

        settings.endGroup();
    }

    groups.clear();
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        groups << SETTINGSGROUP_FILTERWHEEL(i);
    }

    QStringList filterNames;
    std::fill_n(std::back_inserter(filterNames), 6, "empty");

    for (const QString &group : groups) {
        settings.beginGroup(group);

        SET_VALUE(group, SETTING_SERIALNUMBER, QString());
        SET_VALUE(group, SETTING_FILTER_LIST, filterNames);

        settings.endGroup();
    }

    groups.clear();
    for (int i = 0; i < SPIM_NAOTF; ++i) {
        groups << SETTINGSGROUP_AOTF(i);
    }
    for (const QString &group : groups) {
        settings.beginGroup(group);

        SET_VALUE(group, SETTING_SERIALNUMBER, QString());

        settings.endGroup();
    }

    //////////////////////////////////////

    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        PIDevice *dev = spim().getPIDevice(i);
        group = SETTINGSGROUP_AXIS(i);
        dev->setBaud(value(group, SETTING_BAUD).toInt());
        dev->setDeviceNumber(value(group, SETTING_DEVICENUMBER).toInt());
        QString sn = value(group, SETTING_SERIALNUMBER).toString();
        if (!sn.isEmpty()) {
            QSerialPortInfo info = SerialPort::findPortFromSerialNumber(sn);
            if (!info.portName().isEmpty()) {
                dev->setPortName(info.portName());
            }
        } else {
            dev->setPortName(value(group, SETTING_PORTNAME).toString());
        }

        SPIM_PI_DEVICES d_enum = static_cast<SPIM_PI_DEVICES>(i);

        QList<double> *scanRange = spim().getScanRange(d_enum);
        scanRange->replace(0, value(group, SETTING_FROM).toDouble());
        scanRange->replace(1, value(group, SETTING_TO).toDouble());
        scanRange->replace(2, value(group, SETTING_STEP).toDouble());

        spim().setMosaicStageEnabled(d_enum, value(group, SETTING_MOSAIC_ENABLED).toBool());
    }

    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        Cobolt *dev = spim().getLaser(i);
        group = SETTINGSGROUP_COBOLT(i);
        dev->serialPort()->setPortName(value(group, SETTING_PORTNAME).toString());
    }

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        FilterWheel *dev = spim().getFilterWheel(i);
        group = SETTINGSGROUP_FILTERWHEEL(i);
        dev->serialPort()->setPortBySerialNumber(value(group, SETTING_SERIALNUMBER).toString());
    }

    GalvoRamp *gr = spim().getTasks()->getGalvoRamp();
    group = SETTINGSGROUP_GRAMP;
    gr->setPhysicalChannels(value(group, SETTING_PHYSCHANS).toStringList());
    QVector<double> wp;
    const QList<QVariant> wafeformParams = value(group, SETTING_WFPARAMS).toList();
    gr->resetWaveFormParams(SPIM_NCAMS);
    for (int i = 0; i < wafeformParams.count(); i++) {
        wp << wafeformParams.at(i).toDouble();
    }
    gr->setWaveformParams(wp);

    for (int i = 0; i < SPIM_NAOTF; ++i) {
        AA_MPDSnCxx *dev = spim().getAOTF(i);
        group = SETTINGSGROUP_AOTF(i);
        dev->serialPort()->setPortBySerialNumber(value(group, SETTING_SERIALNUMBER).toString());
    }

    group = SETTINGSGROUP_CAMTRIG;
    CameraTrigger *ct = spim().getTasks()->getCameraTrigger();
    ct->setPulseTerms(value(group, SETTING_PULSE_TERMS).toStringList());
    ct->setBlankingPulseTerms(value(group, SETTING_BLANKING_TERMS).toStringList());
    ct->setStartTriggerTerm(value(group, SETTING_TRIGGER_TERM).toString());

    group = SETTINGSGROUP_ACQUISITION;
    spim().setExposureTime(value(group, SETTING_EXPTIME).toDouble());
    spim().setRunName(value(group, SETTING_RUN_NAME).toString());
    spim().setBinning(value(group, SETTING_BINNING).toUInt());

#endif

    group = SETTINGSGROUP_OTHERSETTINGS;
#ifdef MASTER_SPIM
    spim().setScanVelocity(value(group, SETTING_SCANVELOCITY).toDouble());
#endif
    spim().setOutputPathList(value(group, SETTING_CAM_OUTPUT_PATH_LIST).toStringList());
    spim().setTurnOffLasersAtEndOfAcquisition(
        value(group, SETTING_TURN_OFF_LASERS_AT_END_OF_ACQUISITION).toBool());
}

void Settings::saveSettings()
{
    QString group;

#ifdef MASTER_SPIM
    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        PIDevice *dev = spim().getPIDevice(i);
        group = SETTINGSGROUP_AXIS(i);
        setValue(group, SETTING_BAUD, dev->getBaud());
        setValue(group, SETTING_DEVICENUMBER, dev->getDeviceNumber());
        setValue(group, SETTING_PORTNAME, dev->getPortName());
        if (!dev->getPortName().isEmpty()) {
            QSerialPortInfo info(dev->getPortName());
            setValue(group, SETTING_SERIALNUMBER, info.serialNumber());
        }

        SPIM_PI_DEVICES d_enum = static_cast<SPIM_PI_DEVICES>(i);
        QList<double> *scanRange = spim().getScanRange(d_enum);
        setValue(group, SETTING_FROM, scanRange->at(0));
        setValue(group, SETTING_TO, scanRange->at(1));
        setValue(group, SETTING_STEP, scanRange->at(2));

        setValue(group, SETTING_MOSAIC_ENABLED, spim().isMosaicStageEnabled(d_enum));
    }

    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        SerialPort *sp = spim().getLaser(i)->serialPort();
        group = SETTINGSGROUP_COBOLT(i);
        setValue(group, SETTING_PORTNAME, sp->portName());
    }

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        SerialPort *sp = spim().getFilterWheel(i)->serialPort();
        group = SETTINGSGROUP_FILTERWHEEL(i);
        setValue(group, SETTING_SERIALNUMBER, sp->portInfo().serialNumber());
    }

    for (int i = 0; i < SPIM_NAOTF; ++i) {
        SerialPort *sp = spim().getAOTF(i)->serialPort();
        group = SETTINGSGROUP_AOTF(i);
        QString sn = sp->portInfo().serialNumber();
        if (!sn.isEmpty()) {
            setValue(group, SETTING_SERIALNUMBER, sn);
        }
    }

    GalvoRamp *gr = spim().getTasks()->getGalvoRamp();
    group = SETTINGSGROUP_GRAMP;
    setValue(group, SETTING_PHYSCHANS, gr->getPhysicalChannels());
    QList<QVariant> waveformParams;
    for (QVariant variant : gr->getWaveformParams()) {
        waveformParams.append(variant.toDouble());
    }
    setValue(group, SETTING_WFPARAMS, waveformParams);
    CameraTrigger *ct = spim().getTasks()->getCameraTrigger();
    group = SETTINGSGROUP_CAMTRIG;
    setValue(group, SETTING_PULSE_TERMS, ct->getPulseTerms());
    setValue(group, SETTING_BLANKING_TERMS, ct->getBlankingPulseTerms());
    setValue(group, SETTING_TRIGGER_TERM, ct->getStartTriggerTerm());

    group = SETTINGSGROUP_ACQUISITION;
    setValue(group, SETTING_EXPTIME, spim().getExposureTime());
    setValue(group, SETTING_RUN_NAME, spim().getRunName());
    setValue(group, SETTING_BINNING, spim().getBinning());
#endif
    group = SETTINGSGROUP_OTHERSETTINGS;
#ifdef MASTER_SPIM
    setValue(group, SETTING_SCANVELOCITY, spim().getScanVelocity());
#endif
    setValue(group, SETTING_CAM_OUTPUT_PATH_LIST, spim().getOutputPathList());
    setValue(group,
             SETTING_TURN_OFF_LASERS_AT_END_OF_ACQUISITION,
             spim().getTurnOffLasersAtEndOfAcquisition());

    QSettings settings;

    QMapIterator<QString, SettingsMap *> groupIt(map);

    while (groupIt.hasNext()) {
        groupIt.next();
        const SettingsMap *map = groupIt.value();
        QMapIterator<QString, QVariant> it(*map);

        settings.beginGroup(groupIt.key());
        while (it.hasNext()) {
            it.next();
            settings.setValue(it.key(), it.value());
        }
        settings.endGroup();
    }
}

Settings &settings()
{
    static auto instance = std::make_unique<Settings>();
    return *instance;
}
