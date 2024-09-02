// ModbusWorker.cpp
#include "modbusworker.h"
#include "qserialport.h"
#include "qvariant.h"
#include <QDebug>

ModbusWorker::ModbusWorker(const QString &portName, QObject *parent)
    : QObject(parent), portName(portName)
{
    modbusDevice = new QModbusRtuSerialMaster(this); // Use QModbusRtuSerialMaster

    if (!connectToDevice()) {
        emit errorOccurred("Failed to connect to Modbus device.");
    }
}

ModbusWorker::~ModbusWorker()
{
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        modbusDevice->disconnectDevice();
    }
}

bool ModbusWorker::connectToDevice()
{
    modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, portName);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
    modbusDevice->setTimeout(1000);
    modbusDevice->setNumberOfRetries(3);

    if (!modbusDevice->connectDevice()) {
        qDebug() << "Connection failed:" << modbusDevice->errorString();
        emit errorOccurred("Connection failed: " + modbusDevice->errorString());
        return false;
    }

    qDebug() << "Connected to Modbus device.";
    return true;
}

void ModbusWorker::readRegister(int startAddress, int numberOfEntries, int serverAddress)
{
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    auto *reply = modbusDevice->sendReadRequest(readUnit, serverAddress);

    if (!reply) {
        qDebug() << "Failed to send read request:" << modbusDevice->errorString();
        emit errorOccurred("Read error: " + modbusDevice->errorString());
        return;
    }

    bool connectionSuccess = connect(reply, &QModbusReply::finished, this, &ModbusWorker::onReplyFinished);
    qDebug() << "Connection success:" << connectionSuccess;

    if (!connectionSuccess) {
        qDebug() << "Failed to connect reply signal.";
        reply->deleteLater();
        return;
    }

    qDebug() << "Sending read request...";
}

void ModbusWorker::onReplyFinished()
{
    auto *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) {
        qDebug() << "onReplyFinished: Invalid reply object.";
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        emit dataRead(reply->result());
        qDebug() << "onReplyFinished: Data read successfully.";
    } else {
        emit errorOccurred(reply->errorString());
        qDebug() << "onReplyFinished: Error occurred -" << reply->errorString();
    }

    qDebug() << "Modbus device state after reply:" << modbusDevice->state();
    reply->deleteLater();
    qDebug() << "onReplyFinished: Reply object deleted.";
}
