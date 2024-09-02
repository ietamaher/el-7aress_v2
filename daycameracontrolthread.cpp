#include "daycameracontrolthread.h"
#include <QDebug>

DayCameraControlThread::DayCameraControlThread(QObject *parent)
    : QThread(parent),  cameraSerial(nullptr), abort(false), logFlag(true)
{}

DayCameraControlThread::~DayCameraControlThread()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void DayCameraControlThread::setSerialPort(QSerialPort *serial)
{
     cameraSerial = serial;
    if (cameraSerial){
        emit logMessage(QString("[Day Camera] Serial port set for DayCameraControlThread.").arg(identifier));
    }
    else
    {
        emit logMessage(QString("[Day Camera] Failed to open Serial port.").arg(identifier));

    }
}

void DayCameraControlThread::run()
{
    if (cameraSerial){
        emit logMessage(QString("[Day Camera] DayCameraControlThread started."));
    }
    else
    {
        return;
    }

    // Enter the thread's event loop
    exec();
}

void DayCameraControlThread::sendPanLeft() {
    QByteArray currentCommand;
    currentCommand = viscaControl.createPanLeftCommand();
    {
        QMutexLocker locker(&mutex);  // Lock the mutex to ensure thread-safe access to cameraSerial

        if (cameraSerial && cameraSerial->isOpen()) {
            cameraSerial->write(currentCommand);
            emit commandSent();
            logFlag = true;  // Reset the log flag upon successful command execution
        } else {
            logError("Failed to send Pan Left command. Serial port is not open.");
        }
    }  // Mutex is automatically unlocked here when locker goes out of scope
}

void DayCameraControlThread::sendPanRight() {
    QByteArray currentCommand;
    currentCommand = viscaControl.createPanRightCommand();  // Create the command

    {
        QMutexLocker locker(&mutex);  // Lock the mutex to ensure thread-safe access to cameraSerial

        if (cameraSerial && cameraSerial->isOpen()) {
            cameraSerial->write(currentCommand);
            emit commandSent();
            logFlag = true;  // Reset the log flag upon successful command execution
        } else {
            logError("Failed to send Pan Right command. Serial port is not open.");
        }
    }  // Mutex is automatically unlocked here when locker goes out of scope
}

void DayCameraControlThread::sendCommand(const QByteArray &command)
{
    QMutexLocker locker(&mutex);
    currentCommand = command;
}

void DayCameraControlThread::logError(const QString &message) {
    if (logFlag) {
        emit logMessage(QString("[Day Camera] %1").arg(message));
        logFlag = false;  // Disable further logs until the condition changes
    }
}

