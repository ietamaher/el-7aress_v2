#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include "camerawidget.h"

class CameraThread : public QThread {
    Q_OBJECT
public:
    explicit CameraThread(QObject *parent = nullptr);
    ~CameraThread();

protected:
    void run() override;

signals:
    void cameraWidgetReady(CameraWidget *cameraWidget);
    void updateCameraWidgetPosition(const QPoint& position);
    void updateSettingParameters(const QString& burstMode, bool trackingState, bool detectionState, bool stabState, double speed);
    void updateMotorParameters(int incr_, double azimuth);
    void updatePlcParameters(int incr_, bool in1);


public slots:
    void handlePrimaryObjectPositionUpdate(const QPoint& position);
    void handleSettingParameters(const QString& burstMode, bool trackingState, bool detectionState, bool stabState, double speed);
    void handleMotorParameters(int incr_, double azimuth);
    void handlePlcParameters(int incr_, bool in1);


private:
    CameraWidget *cameraWidget;
};

#endif // CAMERATHREAD_H
