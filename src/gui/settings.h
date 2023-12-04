#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

#define SETTINGSGROUP_OTHERSETTINGS "OtherSettings"

#define SETTINGSGROUP_AXIS(n) QString("AXIS_%1").arg(n)
#define SETTINGSGROUP_CAMERAS(n) QString("CAMERA_%1").arg(n)
#define SETTINGSGROUP_FILTERWHEEL(n) QString("FilterWheel_%1").arg(n)

#define SETTING_LUTPATH "LUTPath"
#define SETTING_CAM_OUTPUT_PATH_LIST "camOutputPathList"

#define SETTING_POS "pos"
#define SETTING_VELOCITY "velocity"
#define SETTING_STEPSIZE "stepsize"

#define SETTING_FILTER_LIST "filterList"

typedef QMap<QString, QVariant> SettingsMap;

class Settings
{
public:
    Settings();
    virtual ~Settings();

    QVariant value(const QString &group, const QString &key) const;
    void setValue(const QString &group, const QString &key, const QVariant val);

    void loadSettings();
    void saveSettings();

private:
    QMap<QString, SettingsMap *> map;
};

Settings &settings();

#endif // SETTINGS_H
