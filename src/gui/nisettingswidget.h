#ifndef NISETTINGSWIDGET_H
#define NISETTINGSWIDGET_H

#include <QComboBox>
#include <QWidget>

class NISettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NISettingsWidget(QWidget *parent = nullptr);

signals:

public slots:

private slots:
    void apply();

private:
    void setupUI();

    QList<QComboBox *> galvoRampComboBoxList;
    QList<QComboBox *> aotfBlankingComboBoxList;
    QList<QComboBox *> cameraTriggerCtrComboBoxList;
    QComboBox *PITriggerOutput;
};

#endif // NISETTINGSWIDGET_H
