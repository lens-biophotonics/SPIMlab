#include <memory>

#include <QSettings>
#include <QDir>

#include "core/spim.h"

#include "settings.h"

#define SET_VALUE(group, key, default_val) \
    setValue(group, key, settings.value(key, default_val))

Settings::Settings()
{
    loadSettings();
}

Settings::~Settings()
{
}

QVariant Settings::value(const QString &group, const QString &key) const
{
    if (!map.contains(group) || !map[group]->contains(key)) {
        return QVariant();
    }
    return map[group]->value(key);
}

void Settings::setValue(const QString &group, const QString &key,
                        const QVariant val)
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

    settings.endGroup();

    QStringList groups;
    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        groups << SETTINGSGROUP_AXIS(i);
    }

    for (const QString &group : groups) {
        settings.beginGroup(group);

        QStringList sl;
        sl << SETTING_BAUD
           << SETTING_DEVICENUMBER
           << SETTING_SERIALNUMBER
           << SETTING_PORTNAME
           << SETTING_POS
           << SETTING_FROM
           << SETTING_TO
           << SETTING_STEP
        ;

        for (const QString &s : sl) {
            SET_VALUE(group, s, QVariant());
        }

        SET_VALUE(group, SETTING_STEPSIZE, 0.1);
        SET_VALUE(group, SETTING_VELOCITY, 1.);

        settings.endGroup();
    }


    groupName = SETTINGSGROUP_GRAMP;
    settings.beginGroup(groupName);

    SET_VALUE(groupName,
              SETTING_PHYSCHANS, QStringList({"Dev1/ao2", "Dev1/ao3"}));
    SET_VALUE(groupName, SETTING_WFPARAMS, QList<QVariant>(
                  {0.0, 4.0, 0.0, 0.95, 0.0, 4.0, 0.0, 0.95}));
    SET_VALUE(groupName, SETTING_TRIGGER_TERM, "/Dev1/PFI0");

    settings.endGroup();


    groupName = SETTINGSGROUP_CAMTRIG;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PHYSCHANS,
              QStringList({"/Dev1/port0/line0", "/Dev1/port0/line1"}));
    SET_VALUE(groupName, SETTING_TRIGGER_TERM, "/Dev1/PFI1");

    settings.endGroup();


    groupName = SETTINGSGROUP_ACQUISITION;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_EXPTIME, 0.15);
    SET_VALUE(groupName, SETTING_OUTPUTPATH,
              QDir::toNativeSeparators(QDir::homePath()));

    settings.endGroup();

    groups.clear();
    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        groups << SETTINGSGROUP_COBOLT(i);
    }
    for (int i = 0; i < SPIM_NFILTERWHEEL; ++i) {
        groups << SETTINGSGROUP_FILTERWHEEL(i);
    }

    for (const QString &group : groups) {
        settings.beginGroup(group);

        SET_VALUE(group, SETTING_PORTNAME, QVariant());

        settings.endGroup();
    }
}

void Settings::saveSettings() const
{
    QSettings settings;

    QMapIterator<QString, SettingsMap*> groupIt(map);

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
