#include "nightcameracontrolthread.h"
#include <QDebug>

NightCameraControlThread::NightCameraControlThread(QObject *parent)
    : QThread(parent), cameraSerial(nullptr), abort(false), dev(16), baud(921600)
{}

NightCameraControlThread::~NightCameraControlThread()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
    Close();  // Close the connection to the Boson camera
}

void NightCameraControlThread::setSerialPort(QSerialPort *serial)
{
    cameraSerial = serial;
}

void NightCameraControlThread::run()
{
    // Optionally, initialize the Boson camera
    if (!initializeBosonCamera()) {
        emit errorOccurred("Failed to initialize Boson camera.");
        return;
    }

    exec();
}

bool NightCameraControlThread::initializeBosonCamera()
{
    FLR_RESULT result = Initialize(dev, baud);
    if (result != R_SUCCESS) {
        qCritical() << "Failed to initialize Boson camera. Error code:" << result;
        return false;
    }

    qDebug() << "Boson camera initialized successfully.";
    return true;
}

void NightCameraControlThread::sendPanLeft()
{
    currentCommand = viscaControl.createPanLeftCommand();
    if (cameraSerial && cameraSerial->isOpen()) {
        cameraSerial->write(currentCommand);
        emit commandSent();
    }
}

void NightCameraControlThread::sendPanRight()
{
    currentCommand = viscaControl.createPanRightCommand();
    if (cameraSerial && cameraSerial->isOpen()) {
        cameraSerial->write(currentCommand);
        emit commandSent();
    }
}

void NightCameraControlThread::getCameraInfo()
{
    FLR_RESULT result;
    uint32_t camera_sn;

    result = bosonGetCameraSN(&camera_sn);
    if (result != R_SUCCESS) {
        emit errorOccurred(QString("Failed to get camera serial number. Error code: %1").arg(result));
        return;
    }

    QString info = QString("Camera Serial Number: %1").arg(camera_sn);
    emit bosonCameraInfoReceived(info);
}

void NightCameraControlThread::getSoftwareVersion()
{
    FLR_RESULT result;
    uint32_t major, minor, patch;

    result = bosonGetSoftwareRev(&major, &minor, &patch);
    if (result != R_SUCCESS) {
        emit errorOccurred(QString("Failed to get software version. Error code: %1").arg(result));
        return;
    }

    QString version = QString("Boson Camera Software Version: %1.%2.%3").arg(major).arg(minor).arg(patch);
    emit bosonCameraInfoReceived(version);
}

void NightCameraControlThread::getCameraSerialNumber()
{
    FLR_RESULT result;
    FLR_BOSON_SENSOR_PARTNUMBER_T part_num;

    result = bosonGetSensorPN(&part_num);
    if (result != R_SUCCESS) {
        emit errorOccurred(QString("Failed to get camera part number. Error code: %1").arg(result));
        return;
    }

    QString partNumber = QString("Camera Part Number: %1")
                             .arg(QString::fromLatin1(reinterpret_cast<const char*>(part_num.value), sizeof(part_num.value)));
    emit bosonCameraInfoReceived(partNumber);
}
