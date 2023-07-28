#include "settings.h"

#include "autofocus.h"
#include "cameratrigger.h"
#include "galvoramp.h"
#include "spim.h"
#include "tasks.h"

#include <memory>

#include <qtlab/hw/pi/pidevice.h>
#include <qtlab/hw/serial/AA_MPDSnCxx.h>
#include <qtlab/hw/serial/cobolt.h>
#include <qtlab/hw/serial/filterwheel.h>
#include <qtlab/hw/serial/serialport.h>

#include <QDir>
#include <QSerialPortInfo>
#include <QSettings>

#include <rapid-af.h>

#define SET_VALUE(group, key, default_val) setValue(group, key, settings.value(key, default_val))

#define SETTINGSGROUP_COBOLT(n) QString("Cobolt_%1").arg(n)
#define SETTINGSGROUP_ACQUISITION "Acquisition"
#define SETTINGSGROUP_AOTF(n) QString("AOTF_%1").arg(n)
#define SETTINGSGROUP_CAMTRIG "CameraTrigger"
#define SETTINGSGROUP_GRAMP "GalvoRamp"
#define SETTINGSGROUP_AUTOFOCUS "Autofocus"

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
#define SETTING_TRIGGER_DELAY "delay"

#define SETTING_EXPTIME "exposureTime"
#define SETTING_FRAME_RATE "frameRate"
#define SETTING_RUN_NAME "runName"
#define SETTING_BINNING "binning"

#define SETTING_MULTITHREADING_ENABLE "multiThreading"
#define SETTING_PADDING "padding"
#define SETTING_AGREEMENT_THRESHOLD "agreementThreshold"
#define SETTING_QUALITY_ENABLE "qualityEnable"
#define SETTING_QUALITY_SIGMA_THRESHOLD "qualitySigma"
#define SETTING_QUALITY_SRATIO_THRESHOLD "qualitySpectralRatioThreshold"
#define SETTING_QUALITY_SRATIO_RADIUS "qualitySpectralRatioRadius"
#define SETTING_QUALITY_SRATIO_THICKNESS "qualitySpectralRatioThickness"
#define SETTING_PREFILTER_ENABLE "prefilterEnable"
#define SETTING_PREFILTER_KERNEL_SIZE "pfKernelSize"
#define SETTING_PREFILTER_SIGMA "pfSigma"
#define SETTING_BINARIZE_ENABLE "binarizeEnable"
#define SETTING_BINARIZE_THRESHOLD "binarizeThreshold"
#define SETTING_DOG_ENABLE "dogEnable"
#define SETTING_DOG_KERNEL_SIZE "dogKernelSize"
#define SETTING_DOG_SIGMA1 "dogSigma1"
#define SETTING_DOG_SIGMA2 "dogSigma2"
#define SETTING_CANNY_ENABLE "cannyEnable"
#define SETTING_CANNY_KERNEL_SIZE "cannyKernelSize"
#define SETTING_CANNY_SIGMA "cannySigma"
#define SETTING_CANNY_ALPHA "cannyAlpha"
#define SETTING_CANNY_BETA "cannyBeta"
#define SETTING_CALIBRATION_MALPHA "calibration_mAlpha"
#define SETTING_CALIBRATION_QALPHA "calibration_qAlpha"
#define SETTING_CALIBRATION_MBETA1 "calibration_mBeta1"
#define SETTING_CALIBRATION_QBETA1 "calibration_mBeta1"
#define SETTING_CALIBRATION_MBETA2 "calibration_mBeta2"
#define SETTING_CALIBRATION_QBETA2 "calibration_mBeta2"
#define SETTING_ENABLED "enabled"
#define SETTING_OUTPUT_ENABLED "outputEnabled"
#define SETTING_RIGHT_ROI "rightRoi"
#define SETTING_LEFT_ROI "leftRoi"

Settings::Settings()
{
    loadSettings();
}

Settings::~Settings()
{
    saveSettings();
}

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
    QString groupName;

    groupName = SETTINGSGROUP_OTHERSETTINGS;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_LUTPATH, "/opt/Fiji.app/luts/");
    SET_VALUE(groupName, SETTING_SCANVELOCITY, 2.0);
    QStringList camOutputPath;
    camOutputPath << "/mnt/dualspim"
                  << "/mnt/dualspim";
    SET_VALUE(groupName, SETTING_CAM_OUTPUT_PATH_LIST, camOutputPath);

    settings.endGroup();

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

    groupName = SETTINGSGROUP_GRAMP;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PHYSCHANS, QStringList({"Dev1/ao2", "Dev1/ao3","Dev1/ao4","Dev1/ao5","Dev1/ao6","Dev1/ao7","Dev1/ao8","Dev1/ao9","Dev1/ao10"}));
    SET_VALUE(groupName,
              SETTING_WFPARAMS,
              QList<QVariant>({0.0, 4.0, 0.0, 0.95, 0.0, 4.0, 0.0, 0.95}));
    settings.endGroup();

    groupName = SETTINGSGROUP_CAMTRIG;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PULSE_TERMS, QStringList({"/Dev1/PFI0", "/Dev1/PFI1"}));
    SET_VALUE(groupName, SETTING_BLANKING_TERMS, QStringList({"/Dev1/PFI2", "/Dev1/PFI3"}));
    SET_VALUE(groupName, SETTING_TRIGGER_TERM, "/Dev1/PFI4");
    SET_VALUE(groupName, SETTING_TRIGGER_DELAY, 0);

    settings.endGroup();

    groupName = SETTINGSGROUP_ACQUISITION;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_EXPTIME, 0.15);
    SET_VALUE(groupName, SETTING_RUN_NAME, QString());
    SET_VALUE(groupName, SETTING_BINNING, 1);

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

    groupName = SETTINGSGROUP_AUTOFOCUS;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_MULTITHREADING_ENABLE, true);
    SET_VALUE(groupName, SETTING_PADDING, 100);
    SET_VALUE(groupName, SETTING_AGREEMENT_THRESHOLD, 5);

    SET_VALUE(groupName, SETTING_PREFILTER_ENABLE, true);
    SET_VALUE(groupName, SETTING_PREFILTER_KERNEL_SIZE, 20);
    SET_VALUE(groupName, SETTING_PREFILTER_SIGMA, 5);

    SET_VALUE(groupName, SETTING_QUALITY_ENABLE, true);
    SET_VALUE(groupName, SETTING_QUALITY_SIGMA_THRESHOLD, 0.1);
    SET_VALUE(groupName, SETTING_QUALITY_SRATIO_THRESHOLD, 1e-4);
    SET_VALUE(groupName, SETTING_QUALITY_SRATIO_RADIUS, 30);
    SET_VALUE(groupName, SETTING_QUALITY_SRATIO_THICKNESS, 5);

    SET_VALUE(groupName, SETTING_BINARIZE_ENABLE, true);
    SET_VALUE(groupName, SETTING_BINARIZE_THRESHOLD, 0.6);

    SET_VALUE(groupName, SETTING_DOG_ENABLE, true);
    SET_VALUE(groupName, SETTING_DOG_KERNEL_SIZE, 100);
    SET_VALUE(groupName, SETTING_DOG_SIGMA1, 5);
    SET_VALUE(groupName, SETTING_DOG_SIGMA2, 10);

    SET_VALUE(groupName, SETTING_CANNY_ENABLE, true);
    SET_VALUE(groupName, SETTING_CANNY_KERNEL_SIZE, 29);
    SET_VALUE(groupName, SETTING_CANNY_SIGMA, 20);
    SET_VALUE(groupName, SETTING_CANNY_ALPHA, 0);
    SET_VALUE(groupName, SETTING_CANNY_BETA, 1);

    SET_VALUE(groupName, SETTING_EXPTIME, 10000);
    SET_VALUE(groupName, SETTING_FRAME_RATE, 5);
    SET_VALUE(groupName, SETTING_CALIBRATION_M, 0);
    SET_VALUE(groupName, SETTING_CALIBRATION_Q, 0);

    SET_VALUE(groupName, SETTING_ENABLED, true);
    SET_VALUE(groupName, SETTING_OUTPUT_ENABLED, true);

    SET_VALUE(groupName, SETTING_LEFT_ROI, cv::Mat());
    SET_VALUE(groupName, SETTING_RIGHT_ROI, cv::Mat());

    settings.endGroup();

    //////////////////////////////////////

    QString group;

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

    GalvoRamp *gr = spim().getTasks()->getGalvoRamp(); // for the two G2 Light sheet generation galvos
    GalvoRamp *smth = spim().getCorrectionGalvos();   // for the other 7 galvos
    group = SETTINGSGROUP_GRAMP;
    
    QStringList SETTING_PHYSCHAN = SETTING_PHYSCHANS;  // making it officially a QStringList to avoid possible problems when i want to access it by index (it might not be necessary)
    
    for (int i = 0; i < SPIM_NCAMS; ++i){
    gr->setPhysicalChannels(value(group, SETTING_PHYSCHAN->at(i)).toStringList());  // only the two first channels of SETTING_PHYSCHANS are selecetd
    }
    
    for (int i =2; i < 7; ++i){
        smth->setPhysicalChannels(value(group, SETTING_PHYSCHAN->at(i)).toStringList());  // the rest of the strings of SETTING_PHYSCHANS are selected
    }
    
    QList<QVariant> SETTING_WFPARAM = SETTING_WFPARAMS;  //making it official just for safety 
    
    QVector<double> wp;
    const QList<QVariant> wafeformParams;
    for (int i = 0; i < SPIM_NCAMS; i++){
         wafeformParams.append( value(group, SETTING_WFPARAM->at(i).toList());
    gr->resetWaveFormParams(SPIM_NCAMS);
    for (int i = 0; i < wafeformParams.count(); i++) {
        wp << wafeformParams.at(i).toDouble();
    }

    QVector<double> ws; // the same operation for the other galvos
    const QList<QVariant> waleformParams;
    for (int i = 2; i < 7; i++){
         waleformParams.append( value(group, SETTING_WFPARAM->at(i).toList());
    smth->resetWaveFormParams(7); 
    for (int i = 0; i < waleformParams.count(); i++) {
        ws << waleformParams.at(i).toDouble();
    }
        
    gr->setWaveformParams(wp);
    smth->setWaveformParams(ws);  

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
    ct->setDelay(value(group, SETTING_TRIGGER_DELAY).toDouble());

    group = SETTINGSGROUP_ACQUISITION;
    spim().setExposureTime(value(group, SETTING_EXPTIME).toDouble());
    spim().setRunName(value(group, SETTING_RUN_NAME).toString());
    spim().setBinning(value(group, SETTING_BINNING).toUInt());

    group = SETTINGSGROUP_OTHERSETTINGS;
    spim().setScanVelocity(value(group, SETTING_SCANVELOCITY).toDouble());
    spim().setOutputPathList(value(group, SETTING_CAM_OUTPUT_PATH_LIST).toStringList());

    group = SETTINGSGROUP_AUTOFOCUS;
    rapid_af::AlignOptions opt;
    Autofocus *af = spim().getAutoFocus();
    opt.multithreading_enable = value(group, SETTING_MULTITHREADING_ENABLE).toBool();
    opt.padding = value(group, SETTING_PADDING).toUInt();
    opt.agreement_threshold = value(group, SETTING_AGREEMENT_THRESHOLD).toDouble();
    opt.prefilter_enable = value(group, SETTING_PREFILTER_ENABLE).toBool();
    opt.prefilter_ksize = value(group, SETTING_PREFILTER_KERNEL_SIZE).toInt();
    opt.prefilter_sigma = value(group, SETTING_PREFILTER_SIGMA).toDouble();
    opt.bin_enable = value(group, SETTING_BINARIZE_ENABLE).toBool();
    opt.bin_threshold = value(group, SETTING_BINARIZE_THRESHOLD).toDouble();
    opt.dog_enable = value(group, SETTING_DOG_ENABLE).toBool();
    opt.dog_ksize = value(group, SETTING_DOG_KERNEL_SIZE).toInt();
    opt.dog_sigma1 = value(group, SETTING_DOG_SIGMA1).toDouble();
    opt.dog_sigma2 = value(group, SETTING_DOG_SIGMA2).toDouble();
    opt.canny_enable = value(group, SETTING_CANNY_ENABLE).toBool();
    opt.canny_ksize = value(group, SETTING_CANNY_KERNEL_SIZE).toInt();
    opt.canny_sigma = value(group, SETTING_CANNY_SIGMA).toDouble();
    opt.canny_alpha = value(group, SETTING_CANNY_ALPHA).toDouble();
    opt.canny_beta = value(group, SETTING_CANNY_BETA).toDouble();
    af->setOptions(opt);

    rapid_af::ImageQualityOptions iqOpt;
    iqOpt.sigma_threshold = value(group, SETTING_QUALITY_SIGMA_THRESHOLD).toDouble();
    iqOpt.sratio_threshold = value(group, SETTING_QUALITY_SRATIO_THRESHOLD).toDouble();
    iqOpt.sratio_radius = value(group, SETTING_QUALITY_SRATIO_RADIUS).toInt();
    iqOpt.sratio_thickness = value(group, SETTING_QUALITY_SRATIO_THICKNESS).toInt();

    af->setIqOptions(iqOpt);
    af->setImageQualityEnabled(value(group, SETTING_QUALITY_ENABLE).toBool());
    af->setExposureTime_us(value(group, SETTING_EXPTIME).toDouble());
    af->setFrameRate(value(group, SETTING_FRAME_RATE).toDouble());
    af->setCalibration(value(group, SETTING_CALIBRATION_M).toDouble(),
                       value(group, SETTING_CALIBRATION_Q).toDouble());
    af->setEnabled(value(group, SETTING_ENABLED).toBool());
    af->setOutputEnabled(value(group, SETTING_OUTPUT_ENABLED).toBool());
    af->setImage1(value(group, SETTING_LEFT_ROI).toMat());  //maybe the function is no longuer needed ; also not sure if toMat can be used (it was toRect before) 
    af->setImage2(value(group, SETTING_RIGHT_ROI).toMat());
}

void Settings::saveSettings()
{
    QString group;

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
    
    group = SETTINGSGROUP_GRAMP;
    QList<QVariant> waveformParams;
    QStringList SETTING_PHYSCHAN = SETTING_PHYSCHANS;
    
    GalvoRamp *gr = spim().getTasks()->getGalvoRamp(); 
    for(int i =0, i<SPIM_NCAMS, i++){
        setValue(group, SETTING_PHYSCHAN->at(i), gr->getPhysicalChannels());}  
    for (QVariant variant : gr->getWaveformParams()) {
        waveformParams.append(variant.toDouble());
    }
    GalvoRamp *smth = spim().getCorrectionGalvos();
    for(int i =2, i<7, i++){
    setValue(group, SETTING_PHYSCHAN->at(i), smth->getPhysicalChannels());}  
    for (QVariant variantt : smth->getWaveformParams()) {  //doing the same for smth
        waveformParams.append(variantt.toDouble());
    }        
    setValue(group, SETTING_WFPARAM, waveformParams);
    
    CameraTrigger *ct = spim().getTasks()->getCameraTrigger();
    group = SETTINGSGROUP_CAMTRIG;
    setValue(group, SETTING_PULSE_TERMS, ct->getPulseTerms());
    setValue(group, SETTING_BLANKING_TERMS, ct->getBlankingPulseTerms());
    setValue(group, SETTING_TRIGGER_TERM, ct->getStartTriggerTerm());
    setValue(group, SETTING_TRIGGER_DELAY, ct->getDelay());

    group = SETTINGSGROUP_ACQUISITION;
    setValue(group, SETTING_EXPTIME, spim().getExposureTime());
    setValue(group, SETTING_RUN_NAME, spim().getRunName());
    setValue(group, SETTING_BINNING, spim().getBinning());

    group = SETTINGSGROUP_OTHERSETTINGS;
    setValue(group, SETTING_SCANVELOCITY, spim().getScanVelocity());
    setValue(group, SETTING_CAM_OUTPUT_PATH_LIST, spim().getOutputPathList());

    Autofocus *af = spim().getAutoFocus();
    rapid_af::AlignOptions opt = af->getOptions();
    group = SETTINGSGROUP_AUTOFOCUS;
    setValue(group, SETTING_MULTITHREADING_ENABLE, opt.multithreading_enable);
    setValue(group, SETTING_PADDING, opt.padding);
    setValue(group, SETTING_AGREEMENT_THRESHOLD, opt.agreement_threshold);
    setValue(group, SETTING_PREFILTER_ENABLE, opt.prefilter_enable);
    setValue(group, SETTING_PREFILTER_KERNEL_SIZE, opt.prefilter_ksize);
    setValue(group, SETTING_PREFILTER_SIGMA, opt.prefilter_sigma);
    setValue(group, SETTING_BINARIZE_ENABLE, opt.bin_enable);
    setValue(group, SETTING_BINARIZE_THRESHOLD, opt.bin_threshold);
    setValue(group, SETTING_DOG_ENABLE, opt.dog_enable);
    setValue(group, SETTING_DOG_KERNEL_SIZE, opt.dog_ksize);
    setValue(group, SETTING_DOG_SIGMA1, opt.dog_sigma1);
    setValue(group, SETTING_DOG_SIGMA2, opt.dog_sigma2);
    setValue(group, SETTING_CANNY_ENABLE, opt.canny_enable);
    setValue(group, SETTING_CANNY_KERNEL_SIZE, opt.canny_ksize);
    setValue(group, SETTING_CANNY_SIGMA, opt.canny_sigma);
    setValue(group, SETTING_CANNY_ALPHA, opt.canny_alpha);
    setValue(group, SETTING_CANNY_BETA, opt.canny_beta);

    rapid_af::ImageQualityOptions iqOpt = af->getIqOptions();
    setValue(group, SETTING_QUALITY_ENABLE, af->isImageQualityEnabled());
    setValue(group, SETTING_QUALITY_SIGMA_THRESHOLD, iqOpt.sigma_threshold);
    setValue(group, SETTING_QUALITY_SRATIO_THRESHOLD, iqOpt.sratio_threshold);
    setValue(group, SETTING_QUALITY_SRATIO_RADIUS, iqOpt.sratio_radius);
    setValue(group, SETTING_QUALITY_SRATIO_THICKNESS, iqOpt.sratio_thickness);

    setValue(group, SETTING_EXPTIME, af->getExposureTime_us());
    setValue(group, SETTING_FRAME_RATE, af->getFrameRate());

    setValue(group, SETTING_CALIBRATION_MALPHA, af->getCalibration_mAlpha());
    setValue(group, SETTING_CALIBRATION_QALPHA, af->getCalibration_qAlpha());

    setValue(group, SETTING_CALIBRATION_MBETA1, af->getCalibration_mBeta1());
    setValue(group, SETTING_CALIBRATION_QBETA1, af->getCalibration_qBeta1());

    setValue(group, SETTING_CALIBRATION_MBETA2, af->getCalibration_mBeta2());
    setValue(group, SETTING_CALIBRATION_QBETA2, af->getCalibration_qBeta2());

    setValue(group, SETTING_ENABLED, af->isEnabled());
    setValue(group, SETTING_OUTPUT_ENABLED, af->isOutputEnabled());

    setValue(group, SETTING_LEFT_ROI, af->getImage1());
    setValue(group, SETTING_RIGHT_ROI, af->getImage2());

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
