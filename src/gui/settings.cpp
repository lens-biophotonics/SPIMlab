#include <memory>

#include <QSettings>
#include <QtDebug>

#include "settings.h"

Settings::Settings()
{
    loadSettings();
}

Settings::~Settings()
{
    saveSettings();
}

QVariant Settings::value(const QString &key) const
{
    if (!map.contains(key)) {
        return QVariant();
    }
    return map[key];
}

void Settings::setValue(const QString &key, const QVariant val)
{
    map[key] = val;
}

void Settings::loadSettings()
{
    QSettings settings;
    settings.beginGroup("OtherSettings");

    map["LUTPath"] = settings.value("LUTPath", "/opt/Fiji.app/luts/");

    settings.endGroup();
}

void Settings::saveSettings()
{
    QSettings settings;
    settings.beginGroup("OtherSettings");

    QMapIterator<QString, QVariant> it(map);

    while (it.hasNext()) {
        it.next();
        settings.setValue(it.key(), it.value());
    }

    settings.endGroup();
}

Settings &settings()
{
    static auto instance = std::make_unique<Settings>();
    return *instance;
}
