#include "camerathread.h"

CameraThread::CameraThread(QObject *parent) : QThread(parent) {
    cameraWidget = new CameraWidget();
    connect(this, &CameraThread::updateCameraWidgetPosition, cameraWidget, &CameraWidget::setTargetPosition, Qt::QueuedConnection);
    connect(this, &CameraThread::updateSettingParameters, cameraWidget, &CameraWidget::setSettingParameters);
    connect(this, &CameraThread::updateMotorParameters, cameraWidget, &CameraWidget::setMotorParameters);
    connect(this, &CameraThread::updatePlcParameters, cameraWidget, &CameraWidget::setPlcParameters);

}

CameraThread::~CameraThread() {
    delete cameraWidget;
}

void CameraThread::run() {
    // Start camera processing or any other initialization
    cameraWidget->startCamera();

    // Emit signal to pass the CameraWidget instance to the main thread
    emit cameraWidgetReady(cameraWidget);
    exec();
}

void CameraThread::handlePrimaryObjectPositionUpdate(const QPoint& position) {
    emit updateCameraWidgetPosition(position);
}

void CameraThread::handleSettingParameters(const QString& burstMode, bool trackingState, bool detectionState, bool stabState, double speed){
    emit updateSettingParameters(burstMode, trackingState, detectionState, stabState, speed);
}

void CameraThread::handleMotorParameters(int incr_, double azimuth){
    emit updateMotorParameters(incr_, azimuth);
}

void CameraThread::handlePlcParameters(int incr_, bool in1){
    emit updatePlcParameters(incr_, in1);
}
