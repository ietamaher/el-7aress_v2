#include "gpssniffthread.h"
#include <QDebug>

GPSSniffThread::GPSSniffThread(QObject *parent)
    : QThread(parent), serialPort(nullptr), abort(false)
{}

GPSSniffThread::~GPSSniffThread() {
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void GPSSniffThread::setSerialPort(QSerialPort *serial) {
    serialPort = serial;
}

void GPSSniffThread::run() {
    while (!abort) {
        readNMEA();
        QThread::msleep(100);  // Adjust based on the expected data rate
    }
}

void GPSSniffThread::readNMEA() {
    if (serialPort && serialPort->isOpen()) {
        while (serialPort->canReadLine()) {
            QByteArray line = serialPort->readLine();
            QString nmeaData(line.trimmed());
            emit nmeaDataReceived(nmeaData);
        }
    }
}

