#ifndef NISETTINGSWIDGET_H
#define NISETTINGSWIDGET_H

#include <QWidget>

class NISettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NISettingsWidget(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // NISETTINGSWIDGET_H
