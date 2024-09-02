#include "servoactuatorcontrolthread.h"
#include <QDebug>

ServoActuatorControlThread::ServoActuatorControlThread(QObject *parent)
    : QThread(parent), servoSerial(nullptr), abort(false)
{

}

ServoActuatorControlThread::~ServoActuatorControlThread() {
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void ServoActuatorControlThread::setSerialPort(QSerialPort *serial) {
    servoSerial = serial;
}

void ServoActuatorControlThread::run() {
    exec();
}

QString ServoActuatorControlThread::sendCommand(const QString &command) {
    if (servoSerial && servoSerial->isOpen()) {
        // Send the command
        servoSerial->write((command + "\r").toUtf8());
        servoSerial->waitForBytesWritten(100);

        // Wait for and read the response
        if (servoSerial->waitForReadyRead(1000)) {
            QByteArray response = servoSerial->readAll();
            emit responseReceived(QString(response).trimmed());
            return QString(response).trimmed();
        }
    }
    return QString();
}

void ServoActuatorControlThread::moveToPosition(int position) {
    QString command = QString("TA %1").arg(position);
    sendCommand(command);
    emit commandSent();
}

void ServoActuatorControlThread::checkStatus() {
    QString command = "STATUS";  // Assuming "STATUS" is the command to check status
    sendCommand(command);
    emit commandSent();
}

void ServoActuatorControlThread::checkAlarms() {
    QString command = "ALARM";  // Assuming "ALARM" is the command to check for alarms
    sendCommand(command);
    emit commandSent();
}

