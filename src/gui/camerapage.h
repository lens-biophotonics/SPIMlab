#ifndef CAMERAPAGE_H
#define CAMERAPAGE_H

#include <QWidget>
#include <qtlab/widgets/pipositioncontrolwidget.h>


class CameraPage : public QWidget
{
    Q_OBJECT
public:
    explicit CameraPage(QWidget *parent = nullptr);
    virtual ~CameraPage();

signals:

public slots:

private:
    void setupUI();
    void saveSettings();

    PIPositionControlWidget *stageCw;
};

#endif // CAMERAPAGE_H
