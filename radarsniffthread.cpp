#include "radarsniffthread.h"
#include <QDebug>

RadarSniffThread::RadarSniffThread(QObject *parent)
    : QThread(parent), serialPort(nullptr), abort(false)
{}

RadarSniffThread::~RadarSniffThread() {
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void RadarSniffThread::setSerialPort(QSerialPort *serial) {
    serialPort = serial;
}

void RadarSniffThread::run() {
    while (!abort) {
        readNMEA();
        QThread::msleep(100);  // Adjust based on the expected data rate
    }
}

void RadarSniffThread::readNMEA() {
    if (serialPort && serialPort->isOpen()) {
        while (serialPort->canReadLine()) {
            QByteArray line = serialPort->readLine();
            QString nmeaData(line.trimmed());
            emit nmeaDataReceived(nmeaData);
        }
    }
}

