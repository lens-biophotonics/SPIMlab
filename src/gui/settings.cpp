#include <memory>

#include <QSettings>

#include "core/serialport.h"

#include "settings.h"

#define SET_VALUE(group, key, default_val) \
    setValue(group, key, settings.value(key, default_val))

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
    QSettings settings;
    QString groupName;

    groupName = SETTINGSGROUP_OTHERSETTINGS;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_LUTPATH, "/opt/Fiji.app/luts/");

    settings.endGroup();

    QStringList groups;
    for (int i = 0; i < 5; ++i) {
        groups << SETTINGSGROUP_AXIS(i);
    }

    for (const QString &group : groups) {
        settings.beginGroup(group);

        QStringList sl;
        sl << SETTING_BAUD
           << SETTING_DEVICENUMBER
           << SETTING_SERIALNUMBER
           << SETTING_PORTNAME
        ;

        for (const QString &s : sl) {
            SET_VALUE(group, s, QVariant());
        }

        settings.endGroup();
    }

    groupName = SETTINGSGROUP_GRAMP;
    settings.beginGroup(groupName);

    SET_VALUE(groupName,
              SETTING_PHYSCHANS, QStringList({"Dev1/ao0", "Dev1/ao1"}));
    SET_VALUE(groupName, SETTING_WFPARAMS, QList<QVariant>(
                  {0.2, 2., 0., 100., 0.2, 2., 0.5, 100.}));

    settings.endGroup();


    groupName = SETTINGSGROUP_CAMTRIG;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PHYSCHANS,
              QStringList({"Dev1/ctr0", "Dev1/ctr1"}));
    SET_VALUE(groupName,
              SETTING_TERMS, QStringList({"/Dev1/PFI0", "/Dev1/PFI1"}));
    SET_VALUE(groupName, SETTING_TRIGGER_TERM, QVariant());

    settings.endGroup();


    groupName = SETTINGSGROUP_SPIM;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_EXPTIME, 0.1);

    settings.endGroup();
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
