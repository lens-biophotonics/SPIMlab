#ifndef CAMERAPAGE_H
#define CAMERAPAGE_H

#include <QWidget>

class CameraPage : public QWidget
{
    Q_OBJECT
public:
    explicit CameraPage(QWidget *parent = nullptr);

signals:

public slots:

private:
    void setupUI();
};

#endif // CAMERAPAGE_H
