// lrfcontrolthread.cpp
#include "lrfcontrolthread.h"
#include <QDebug>

LRFControlThread::LRFControlThread(QObject *parent)
    : QThread(parent), lrfSerial(nullptr), abort(false)
{}

LRFControlThread::~LRFControlThread() {
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void LRFControlThread::setSerialPort(QSerialPort *serial) {
    lrfSerial = serial;
}

void LRFControlThread::run() {
    exec();
}

quint8 LRFControlThread::calculateChecksum(const QByteArray &command) {
    quint8 checksum = 0;
    for (int i = 3; i < command.size(); ++i) {
        checksum += static_cast<quint8>(command[i]);
    }
    return checksum & 0xFF;
}

QByteArray LRFControlThread::buildCommand(const QByteArray &commandTemplate) {
    QByteArray command = commandTemplate;
    quint8 checksum = calculateChecksum(command);
    command.append(checksum);
    return command;
}

QByteArray LRFControlThread::sendCommand(const QByteArray &command) {
    if (lrfSerial && lrfSerial->isOpen()) {
        lrfSerial->write(command);
        lrfSerial->waitForBytesWritten(100);
        lrfSerial->waitForReadyRead(100);
        QByteArray response = lrfSerial->readAll();
        emit responseReceived(response);
        return response;
    }
    return QByteArray();
}

void LRFControlThread::sendSelfCheck() {
    QByteArray commandTemplate = QByteArray::fromHex("EE16020301");
    QByteArray command = buildCommand(commandTemplate);
    sendCommand(command);
    emit commandSent();
}

void LRFControlThread::sendSingleRanging() {
    QByteArray commandTemplate = QByteArray::fromHex("EE16020302");
    QByteArray command = buildCommand(commandTemplate);
    sendCommand(command);
    emit commandSent();
}

void LRFControlThread::sendContinuousRanging() {
    QByteArray commandTemplate = QByteArray::fromHex("EE16020304");
    QByteArray command = buildCommand(commandTemplate);
    sendCommand(command);
    emit commandSent();
}

void LRFControlThread::stopRanging() {
    QByteArray commandTemplate = QByteArray::fromHex("EE16020305");
    QByteArray command = buildCommand(commandTemplate);
    sendCommand(command);
    emit commandSent();
}
