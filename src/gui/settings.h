#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

#define SETTINGSGROUP_OTHERSETTINGS "OtherSettings"
#define SETTINGSGROUP_SPIM "SPIM"

#define SETTINGSGROUP_AXIS(n) QString("AXIS_%1").arg(n)

#define SETTINGSGROUP_CAMTRIG "CameraTrigger"
#define SETTINGSGROUP_GRAMP "GalvoRamp"

#define SETTING_LUTPATH "LUTPath"

#define SETTING_BAUD "baud"
#define SETTING_DEVICENUMBER "deviceNumber"
#define SETTING_SERIALNUMBER "serialNumber"
#define SETTING_PORTNAME "portName"

#define SETTING_PHYSCHANS "physicalChannels"
#define SETTING_WFPARAMS "waveformParams"

#define SETTING_TERMS "terms"

#define SETTING_EXPTIME "exposureTime"

typedef QMap<QString, QVariant> SettingsMap;

class Settings
{
public:
    Settings();
    virtual ~Settings();

    QVariant value(const QString &group, const QString &key) const;
    void setValue(const QString &group, const QString &key, const QVariant val);

private:
    QMap<QString, SettingsMap*> map;

    void loadSettings();
    void saveSettings() const;
};

Settings& settings();

#endif // SETTINGS_H
