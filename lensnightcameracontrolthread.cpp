#include "lensnightcameracontrolthread.h"
#include <QDebug>

LensNightCameraControlThread::LensNightCameraControlThread(QObject *parent)
    : QThread(parent), lensSerial(nullptr), abort(false)
{}

LensNightCameraControlThread::~LensNightCameraControlThread() {
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void LensNightCameraControlThread::setSerialPort(QSerialPort *serial) {
    lensSerial = serial;
}

void LensNightCameraControlThread::run() {
    exec();
}

QString LensNightCameraControlThread::sendCommand(const QString &command) {
    if (lensSerial && lensSerial->isOpen()) {
        // Send the command with a carriage return
        lensSerial->write((command + "\r").toUtf8());
        lensSerial->waitForBytesWritten(100);

        // Wait for and read the response
        if (lensSerial->waitForReadyRead(1000)) {
            QByteArray response = lensSerial->readAll();
            emit responseReceived(QString(response).trimmed());
            return QString(response).trimmed();
        }
    }
    return QString();
}

void LensNightCameraControlThread::moveToWFOV() {
    QString command = "/MPAv 0, p";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::moveToNFOV() {
    QString command = "/MPAv 100, p";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::moveToIntermediateFOV(int percentage) {
    QString command = QString("/MPAv %1, p").arg(percentage);
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::moveToFocalLength(int efl) {
    QString command = QString("/MPAv %1, F").arg(efl);
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::moveToInfinityFocus() {
    QString command = "/MPAf 100, u";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::moveFocusNear(int amount) {
    QString command = QString("/MPRf %1").arg(-amount);
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::moveFocusFar(int amount) {
    QString command = QString("/MPRf %1").arg(amount);
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::getFocusPosition() {
    QString command = "/GMSf[2] 1";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::getLensTemperature() {
    QString command = "/GTV";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::resetController() {
    QString command = "/RST0 NEOS";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::homeAxis(int axis) {
    QString command = QString("/HOM%1").arg(axis);
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::turnOnTemperatureCompensation() {
    QString command = "/MDF[4] 2";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::turnOffTemperatureCompensation() {
    QString command = "/MDF[4] 0";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::turnOnRangeCompensation() {
    QString command = "/MDF[5] 2";
    sendCommand(command);
    emit commandSent();
}

void LensNightCameraControlThread::turnOffRangeCompensation() {
    QString command = "/MDF[5] 0";
    sendCommand(command);
    emit commandSent();
}
