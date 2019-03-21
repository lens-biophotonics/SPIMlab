#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

class Settings
{
public:
    Settings();
    virtual ~Settings();

    QVariant value(const QString &key) const;
    void setValue(const QString &key, const QVariant val);

private:
    QMap<QString, QVariant> map;

    void loadSettings();
    void saveSettings();
};

Settings& settings();

#endif // SETTINGS_H
